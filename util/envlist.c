#include "qemu/osdep.h"
#include "qemu/queue.h"
#include "qemu/envlist.h"

struct envlist_entry {
    const char *ev_var;            /* actual env value */
    QLIST_ENTRY(envlist_entry) ev_link;
};

struct envlist {
    QLIST_HEAD(, envlist_entry) el_entries; /* actual entries */
    size_t el_count;                        /* number of entries */
};

static int envlist_parse(envlist_t *envlist,
    const char *env, int (*)(envlist_t *, const char *));

/*
 * Allocates new envlist and returns pointer to it.
 */
envlist_t *
envlist_create(void)
{
    envlist_t *envlist;

    envlist = g_malloc(sizeof(*envlist));

    QLIST_INIT(&envlist->el_entries);
    envlist->el_count = 0;

    return (envlist);
}

/*
 * Releases given envlist and its entries.
 */
void
envlist_free(envlist_t *envlist)
{
    struct envlist_entry *entry;

    assert(envlist != NULL);

    while (envlist->el_entries.lh_first != NULL) {
        entry = envlist->el_entries.lh_first;
        QLIST_REMOVE(entry, ev_link);

        g_free((char *)entry->ev_var);
        g_free(entry);
    }
    g_free(envlist);
}

/*
 * Parses comma separated list of set/modify environment
 * variable entries and updates given enlist accordingly.
 *
 * For example:
 *     envlist_parse(el, "HOME=foo,SHELL=/bin/sh");
 *
 * inserts/sets environment variables HOME and SHELL.
 *
 * Returns 0 on success, errno otherwise.
 */
int
envlist_parse_set(envlist_t *envlist, const char *env)
{
    return (envlist_parse(envlist, env, &envlist_setenv));
}

/*
 * Parses comma separated list of unset environment variable
 * entries and removes given variables from given envlist.
 *
 * Returns 0 on success, errno otherwise.
 */
int
envlist_parse_unset(envlist_t *envlist, const char *env)
{
    return (envlist_parse(envlist, env, &envlist_unsetenv));
}

/*
 * Parses comma separated list of set, modify or unset entries
 * and calls given callback for each entry.
 *
 * Returns 0 in case of success, errno otherwise.
 */
static int
envlist_parse(envlist_t *envlist, const char *env,
    int (*callback)(envlist_t *, const char *))
{
    char *tmpenv, *envvar;
    char *envsave = NULL;
    int ret = 0;
    assert(callback != NULL);

    if ((envlist == NULL) || (env == NULL))
        return (EINVAL);

    tmpenv = g_strdup(env);
    envsave = tmpenv;

    do {
        envvar = strchr(tmpenv, ',');
        if (envvar != NULL) {
            *envvar = '\0';
        }
        if ((*callback)(envlist, tmpenv) != 0) {
            ret = errno;
            break;
        }
        tmpenv = envvar + 1;
    } while (envvar != NULL);

    g_free(envsave);
    return ret;
}

/*
 * Sets environment value to envlist in similar manner
 * than putenv(3).
 *
 * Returns 0 in success, errno otherwise.
 */
int
envlist_setenv(envlist_t *envlist, const char *env)
{
    struct envlist_entry *entry = NULL;
    const char *eq_sign;
    size_t envname_len;

    if ((envlist == NULL) || (env == NULL))
        return (EINVAL);

    /* find out first equals sign in given env */
    if ((eq_sign = strchr(env, '=')) == NULL)
        return (EINVAL);
    envname_len = eq_sign - env + 1;

    /*
     * If there already exists variable with given name
     * we remove and release it before allocating a whole
     * new entry.
     */
    for (entry = envlist->el_entries.lh_first; entry != NULL;
        entry = entry->ev_link.le_next) {
        if (strncmp(entry->ev_var, env, envname_len) == 0)
            break;
    }

    if (entry != NULL) {
        QLIST_REMOVE(entry, ev_link);
        g_free((char *)entry->ev_var);
        g_free(entry);
    } else {
        envlist->el_count++;
    }

    entry = g_malloc(sizeof(*entry));
    entry->ev_var = g_strdup(env);
    QLIST_INSERT_HEAD(&envlist->el_entries, entry, ev_link);

    return (0);
}

/*
 * Removes given env value from envlist in similar manner
 * than unsetenv(3).  Returns 0 in success, errno otherwise.
 */
int
envlist_unsetenv(envlist_t *envlist, const char *env)
{
    struct envlist_entry *entry;
    size_t envname_len;

    if ((envlist == NULL) || (env == NULL))
        return (EINVAL);

    /* env is not allowed to contain '=' */
    if (strchr(env, '=') != NULL)
        return (EINVAL);

    /*
     * Find out the requested entry and remove
     * it from the list.
     */
    envname_len = strlen(env);
    for (entry = envlist->el_entries.lh_first; entry != NULL;
        entry = entry->ev_link.le_next) {
        if (strncmp(entry->ev_var, env, envname_len) == 0)
            break;
    }
    if (entry != NULL) {
        QLIST_REMOVE(entry, ev_link);
        g_free((char *)entry->ev_var);
        g_free(entry);

        envlist->el_count--;
    }
    return (0);
}

/*
 * Appends environment value to envlist. If the environment
 * variable already exists, the new value is appended to the
 * existing one.
 *
 * Returns 0 in success, errno otherwise.
 */
int
envlist_appendenv(envlist_t *envlist, const char *env, const char *separator)
{
    struct envlist_entry *entry = NULL;
    const char *eq_sign;
    size_t envname_len;

    if ((envlist == NULL) || (env == NULL) || (separator == NULL)) {
        return (EINVAL);
    }

    /* find out first equals sign in given env */
    eq_sign = strchr(env, '=');
    if (eq_sign == NULL) {
        return (EINVAL);
    }

    if (strchr(eq_sign + 1, '=') != NULL) {
        return (EINVAL);
    }

    envname_len = eq_sign - env + 1;

    /*
     * If there already exists variable with given name,
     * we append the new value to the existing one.
     */
    for (entry = envlist->el_entries.lh_first; entry != NULL;
        entry = entry->ev_link.le_next) {
        if (strncmp(entry->ev_var, env, envname_len) == 0) {
            break;
        }
    }

    if (entry != NULL) {
        char *new_env_value = NULL;
        size_t new_env_len = strlen(entry->ev_var) + strlen(eq_sign)
            + strlen(separator) + 1;
        new_env_value = g_malloc(new_env_len);
        strcpy(new_env_value, entry->ev_var);
        strcat(new_env_value, separator);
        strcat(new_env_value, eq_sign + 1);
        g_free((char *)entry->ev_var);
        entry->ev_var = new_env_value;
    } else {
        envlist->el_count++;
        entry = g_malloc(sizeof(*entry));
        entry->ev_var = g_strdup(env);
        QLIST_INSERT_HEAD(&envlist->el_entries, entry, ev_link);
    }

    return (0);
}

/*
 * Returns given envlist as array of strings (in same form that
 * global variable environ is).  Caller must free returned memory
 * by calling g_free for each element and the array.
 * Returned array and given envlist are not related (no common
 * references).
 *
 * If caller provides count pointer, number of items in array is
 * stored there.
 */
char **
envlist_to_environ(const envlist_t *envlist, size_t *count)
{
    struct envlist_entry *entry;
    char **env, **penv;

    penv = env = g_new(char *, envlist->el_count + 1);

    for (entry = envlist->el_entries.lh_first; entry != NULL;
        entry = entry->ev_link.le_next) {
        *(penv++) = g_strdup(entry->ev_var);
    }
    *penv = NULL; /* NULL terminate the list */

    if (count != NULL)
        *count = envlist->el_count;

    return (env);
}

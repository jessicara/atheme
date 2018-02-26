/*
 * Copyright (C) 2005 Jilles Tjoelker, et al.
 * Rights to this code are as documented in doc/LICENSE.
 *
 * Fine grained services operator privileges
 */

#ifndef PRIVS_H
#define PRIVS_H

#define PRIV_NONE            NULL

/* nickserv/userserv */
#define PRIV_USER_AUSPEX     "user:auspex"
#define PRIV_USER_ADMIN      "user:admin"
#define PRIV_USER_SENDPASS   "user:sendpass"
#define PRIV_USER_VHOST      "user:vhost"
#define PRIV_USER_FREGISTER  "user:fregister"
/* chanserv */
#define PRIV_CHAN_AUSPEX     "chan:auspex"
#define PRIV_CHAN_ADMIN      "chan:admin"
#define PRIV_CHAN_CMODES     "chan:cmodes"
#define PRIV_JOIN_STAFFONLY  "chan:joinstaffonly"
/* nickserv/userserv+chanserv */
#define PRIV_MARK            "user:mark"
#define PRIV_HOLD            "user:hold"
#define PRIV_REG_NOLIMIT     "user:regnolimit"
/* generic */
#define PRIV_SERVER_AUSPEX   "general:auspex"
#define PRIV_VIEWPRIVS       "general:viewprivs"
#define PRIV_FLOOD           "general:flood"
#define PRIV_HELPER	     "general:helper"
#define PRIV_METADATA        "general:metadata"
#define PRIV_ADMIN           "general:admin"
/* operserv */
#define PRIV_OMODE           "operserv:omode"
#define PRIV_AKILL           "operserv:akill"
#define PRIV_MASS_AKILL      "operserv:massakill"
#define PRIV_AKILL_ANYMASK   "operserv:akill-anymask"
#define PRIV_JUPE            "operserv:jupe"
#define PRIV_NOOP            "operserv:noop"
#define PRIV_GLOBAL          "operserv:global"
#define PRIV_GRANT           "operserv:grant"
#define PRIV_OVERRIDE        "operserv:override"
/* saslserv */
#define PRIV_IMPERSONATE_CLASS_FMT	"impersonate:class:%s"
#define PRIV_IMPERSONATE_ENTITY_FMT	"impersonate:entity:%s"
#define PRIV_IMPERSONATE_ANY		"impersonate:any"

/* other access levels */
#define AC_NONE NULL /* anyone */
#define AC_DISABLED "special:disabled" /* noone */
#define AC_AUTHENTICATED "special:authenticated"
/* please do not use the following anymore */
#define AC_IRCOP "special:ircop"
#define AC_SRA "general:admin"

struct operclass
{
  char *name;
  char *privs; /* priv1 priv2 priv3... */
  int flags;
  mowgli_node_t node;
};

#define OPERCLASS_NEEDOPER	0x1 /* only give privs to IRCops */
#define OPERCLASS_BUILTIN	0x2 /* builtin */

/* soper list struct */
struct soper_ {
  myuser_t *myuser;
  char *name;
  struct operclass *operclass;
  char *classname;
  int flags;
  char *password;
};

#define SOPER_CONF	0x1 /* oper is listed in atheme.conf */

/* privs.c */
extern mowgli_list_t operclasslist;
extern mowgli_list_t soperlist;

extern void init_privs(void);

extern struct operclass *operclass_add(const char *name, const char *privs, int flags);
extern void operclass_delete(struct operclass *operclass);
extern struct operclass *operclass_find(const char *name);

extern soper_t *soper_add(const char *name, const char *classname, int flags, const char *password);
extern void soper_delete(soper_t *soper);
extern soper_t *soper_find(myuser_t *myuser);
extern soper_t *soper_find_named(const char *name);

extern bool is_soper(myuser_t *myuser);
extern bool is_conf_soper(myuser_t *myuser);

/* has_any_privs(): used to determine whether we should give detailed
 * messages about disallowed things
 * warning: do not use this for any kind of real privilege! */
extern bool has_any_privs(struct sourceinfo *);
extern bool has_any_privs_user(user_t *);
/* has_priv(): for sources of commands */
extern bool has_priv(struct sourceinfo *, const char *);
/* has_priv_user(): for online users */
extern bool has_priv_user(user_t *, const char *);
/* has_priv_myuser(): channel succession etc */
extern bool has_priv_myuser(myuser_t *, const char *);
/* has_priv_operclass(): /os specs etc */
extern bool has_priv_operclass(struct operclass *, const char *);
/* has_all_operclass(): checks if source has all privs in operclass */
extern bool has_all_operclass(struct sourceinfo *, struct operclass *);

/* get_sourceinfo_soper(): get the specific operclass role which is granting
 * privilege authority
 */
extern const soper_t *get_sourceinfo_soper(struct sourceinfo *si);
/* get_sourceinfo_operclass(): get the specific operclass role which is granting
 * privilege authority
 */
extern const struct operclass *get_sourceinfo_operclass(struct sourceinfo *si);

#endif /* PRIVS_H */

/*
 * Copyright (c) 2005 Atheme Development Group
 * Rights to this code are as documented in doc/LICENSE.
 *
 * This file contains code for the nickserv LISTCHANS function.
 *   -- Contains an alias "MYACCESS" for legacy users
 */

#include "atheme.h"

static void ns_cmd_listchans(struct sourceinfo *si, int parc, char *parv[]);

struct command ns_listchans = { "LISTCHANS", N_("Lists channels that you have access to."), AC_NONE, 1, ns_cmd_listchans, { .path = "nickserv/listchans" } };

static void
mod_init(module_t *const restrict m)
{
	service_named_bind_command("nickserv", &ns_listchans);
}

static void
mod_deinit(const module_unload_intent_t intent)
{
	service_named_unbind_command("nickserv", &ns_listchans);
}

static void ns_cmd_listchans(struct sourceinfo *si, int parc, char *parv[])
{
	myuser_t *mu;
	mowgli_node_t *n;
	chanacs_t *ca;
	unsigned int akicks = 0, i;

	/* Optional target */
	char *target = parv[0];

	if (target)
	{
		if (!has_priv(si, PRIV_CHAN_AUSPEX))
		{
			command_fail(si, fault_noprivs, _("You are not authorized to use the target argument."));
			return;
		}

		mu = myuser_find_ext(target);

		if (mu == NULL)
		{
			command_fail(si, fault_nosuch_target, _("\2%s\2 is not registered."), target);
			return;
		}
	}
	else
	{
		mu = si->smu;
		if (mu == NULL)
		{
			command_fail(si, fault_noprivs, _("You are not logged in."));
			return;
		}
	}

	if (mu != si->smu)
	{	/* must have been an oper */
		logcommand(si, CMDLOG_ADMIN, "LISTCHANS: \2%s\2", entity(mu)->name);
	}
	else
	{	/* just a user, or oper is listing himself */
		logcommand(si, CMDLOG_GET, "LISTCHANS");
	}

	if (MOWGLI_LIST_LENGTH(&entity(mu)->chanacs) == 0)
	{
		command_success_nodata(si, _("No channel access was found for the nickname \2%s\2."), entity(mu)->name);
		return;
	}

	MOWGLI_ITER_FOREACH(n, entity(mu)->chanacs.head)
	{
		ca = (chanacs_t *)n->data;

		/* don't tell users they're akicked (flag +b) */
		if (ca->level != CA_AKICK)
			command_success_nodata(si, _("Access flag(s) %s in %s"), bitmask_to_flags(ca->level), ca->mychan->name);
		else
			akicks++;
	}

	i = MOWGLI_LIST_LENGTH(&entity(mu)->chanacs) - akicks;

	if (i == 0)
		command_success_nodata(si, _("No channel access was found for the nickname \2%s\2."), entity(mu)->name);
	else
		command_success_nodata(si, ngettext(N_("\2%d\2 channel access match for the nickname \2%s\2"),
						    N_("\2%d\2 channel access matches for the nickname \2%s\2"), i),
						    i, entity(mu)->name);
}

SIMPLE_DECLARE_MODULE_V1("nickserv/listchans", MODULE_UNLOAD_CAPABILITY_OK)

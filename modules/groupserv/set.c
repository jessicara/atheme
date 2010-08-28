/*
 * Copyright (c) 2003-2004 E. Will et al.
 * Rights to this code are documented in doc/LICENSE.
 *
 * This file contains routines to handle the GroupServ SET commands.
 * For now, don't split out till i can figure out how to create multiple modules
 * in GroupServ.
 *
 */

#include "atheme.h"
#include "groupserv.h"

static void gs_help_set(sourceinfo_t *si);
static void gs_cmd_set(sourceinfo_t *si, int parc, char *parv[]);
static void gs_cmd_set_email(sourceinfo_t *si, int parc, char *parv[]);
static void gs_cmd_set_url(sourceinfo_t *si, int parc, char *parv[]);
static void gs_cmd_set_description(sourceinfo_t *si, int parc, char *parv[]);

command_t gs_set = { "SET", N_("Sets various control flags."), AC_NONE, 3, gs_cmd_set };
command_t gs_set_email = { "EMAIL", N_("Sets the group e-mail address."), AC_NONE, 2, gs_cmd_set_email };
command_t gs_set_url = { "URL", N_("Sets the group URL."), AC_NONE, 2, gs_cmd_set_url };
command_t gs_set_description = { "DESCRIPTION", N_("Sets the group description."), AC_NONE, 2, gs_cmd_set_description };

list_t gs_set_cmdtree;

void set_init(void)
{
	command_add(&gs_set, &gs_cmdtree);
	command_add(&gs_set_email, &gs_set_cmdtree);
	command_add(&gs_set_url, &gs_set_cmdtree);
	command_add(&gs_set_description, &gs_set_cmdtree);

	help_addentry(&gs_helptree, "SET", NULL, gs_help_set);
	help_addentry(&gs_helptree, "SET EMAIL", "help/groupserv/set_email", NULL); 
	help_addentry(&gs_helptree, "SET URL", "help/groupserv/set_url", NULL); 
	help_addentry(&gs_helptree, "SET DESCRIPTION", "help/groupserv/set_description", NULL); 
}

void set_deinit(void)
{
	command_delete(&gs_set, &gs_cmdtree);
	command_delete(&gs_set_email, &gs_set_cmdtree);
	command_delete(&gs_set_url, &gs_set_cmdtree);
	command_delete(&gs_set_description, &gs_set_cmdtree);

	help_delentry(&gs_helptree, "SET");
	help_delentry(&gs_helptree, "SET EMAIL");
	help_delentry(&gs_helptree, "SET URL");
	help_delentry(&gs_helptree, "SET DESCRIPTION");
}

static void gs_help_set(sourceinfo_t *si)
{
	command_success_nodata(si, _("Help for \2SET\2:"));
	command_success_nodata(si, " ");
	command_success_nodata(si, _("SET allows you to set various control flags\n"
				"for groups that change the way certain\n"
				"operations are performed on them."));
	command_success_nodata(si, " ");
	command_help(si, &gs_set_cmdtree);
	command_success_nodata(si, " ");
	command_success_nodata(si, _("For more specific help use \2/msg %s HELP SET \37command\37\2."), si->service->disp);
}

/* SET <!group> <setting> <parameters> */
static void gs_cmd_set(sourceinfo_t *si, int parc, char *parv[])
{
	char *group;
	char *cmd;
	command_t *c;

	if (parc < 2)
	{
		command_fail(si, fault_needmoreparams, STR_INSUFFICIENT_PARAMS, "SET");
		command_fail(si, fault_needmoreparams, _("Syntax: SET <!group> <setting> [parameters]"));
		return;
	}

	if (parv[0][0] == '!')
		group = parv[0], cmd = parv[1];
	else if (parv[1][0] == '!')
		cmd = parv[0], group = parv[1];
	else
	{
		command_fail(si, fault_badparams, STR_INVALID_PARAMS, "SET");
		command_fail(si, fault_badparams, _("Syntax: SET <!group> <setting> [parameters]"));
		return;
	}

	c = command_find(&gs_set_cmdtree, cmd);
	if (c == NULL)
	{
		command_fail(si, fault_badparams, _("Invalid command. Use \2/%s%s help\2 for a command listing."), (ircd->uses_rcommand == false) ? "msg " : "", si->service->disp);
		return;
	}

	parv[1] = group;
	command_exec(si->service, si, c, parc - 1, parv + 1);
}

static void gs_cmd_set_email(sourceinfo_t *si, int parc, char *parv[])
{
	mygroup_t *mg;
	char *mail = parv[1];

	if (!(mg = mygroup_find(parv[0])))
	{
		command_fail(si, fault_nosuch_target, _("Group \2%s\2 does not exist."), parv[0]);
		return;
	}

	if (!groupacs_sourceinfo_has_flag(mg, si, GA_SET))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to execute this command."));
		return;
	}

	if (!mail || !strcasecmp(mail, "NONE") || !strcasecmp(mail, "OFF"))
	{
		if (metadata_find(mg, "email"))
		{
			metadata_delete(mg, "email");
			command_success_nodata(si, _("The e-mail address for group \2%s\2 was deleted."), entity(mg)->name);
			logcommand(si, CMDLOG_SET, "SET:EMAIL:NONE: \2%s\2", entity(mg)->name);
			return;
		}

		command_fail(si, fault_nochange, _("The e-mail address for group \2%s\2 was not set."), entity(mg)->name);
		return;
	}

	if (!validemail(mail))
	{
		command_fail(si, fault_badparams, _("\2%s\2 is not a valid e-mail address."), mail);
		return;
	}

	/* we'll overwrite any existing metadata */
	metadata_add(mg, "email", mail);

	logcommand(si, CMDLOG_SET, "SET:EMAIL: \2%s\2 \2%s\2", entity(mg)->name, mail);
	command_success_nodata(si, _("The e-mail address for group \2%s\2 has been set to \2%s\2."), parv[0], mail);
}

static void gs_cmd_set_url(sourceinfo_t *si, int parc, char *parv[])
{
	mygroup_t *mg;
	char *url = parv[1];

	if (!(mg = mygroup_find(parv[0])))
	{
		command_fail(si, fault_nosuch_target, _("Group \2%s\2 does not exist."), parv[0]);
		return;
	}

	if (!groupacs_sourceinfo_has_flag(mg, si, GA_SET))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to execute this command."));
		return;
	}

	if (!url || !strcasecmp("OFF", url) || !strcasecmp("NONE", url))
	{
		/* not in a namespace to allow more natural use of SET PROPERTY.
		 * they may be able to introduce spaces, though. c'est la vie.
		 */
		if (metadata_find(mg, "url"))
		{
			metadata_delete(mg, "url");
			logcommand(si, CMDLOG_SET, "SET:URL:NONE: \2%s\2", entity(mg)->name);
			command_success_nodata(si, _("The URL for \2%s\2 has been cleared."), parv[0]);
			return;
		}

		command_fail(si, fault_nochange, _("The URL for \2%s\2 was not set."), parv[0]);
		return;
	}

	/* we'll overwrite any existing metadata */
	metadata_add(mg, "url", url);

	logcommand(si, CMDLOG_SET, "SET:URL: \2%s\2 \2%s\2", entity(mg)->name, url);
	command_success_nodata(si, _("The URL of \2%s\2 has been set to \2%s\2."), parv[0], url);
}

static void gs_cmd_set_description(sourceinfo_t *si, int parc, char *parv[])
{
	mygroup_t *mg;
	char *desc = parv[1];

	if (!(mg = mygroup_find(parv[0])))
	{
		command_fail(si, fault_nosuch_target, _("Group \2%s\2 does not exist."), parv[0]);
		return;
	}

	if (!groupacs_sourceinfo_has_flag(mg, si, GA_SET))
	{
		command_fail(si, fault_noprivs, _("You are not authorized to execute this command."));
		return;
	}

	if (!desc || !strcasecmp("OFF", desc) || !strcasecmp("NONE", desc))
	{
		/* not in a namespace to allow more natural use of SET PROPERTY.
		 * they may be able to introduce spaces, though. c'est la vie.
		 */
		if (metadata_find(mg, "description"))
		{
			metadata_delete(mg, "description");
			logcommand(si, CMDLOG_SET, "SET:DESCRIPTION:NONE: \2%s\2", entity(mg)->name);
			command_success_nodata(si, _("The description for \2%s\2 has been cleared."), parv[0]);
			return;
		}

		command_fail(si, fault_nochange, _("A description for \2%s\2 was not set."), parv[0]);
		return;
	}

	/* we'll overwrite any existing metadata */
	metadata_add(mg, "description", desc);

	logcommand(si, CMDLOG_SET, "SET:DESCRIPTION: \2%s\2 \2%s\2", entity(mg)->name, desc);
	command_success_nodata(si, _("The description of \2%s\2 has been set to \2%s\2."), parv[0], desc);
}
/* vim:cinoptions=>s,e0,n0,f0,{0,}0,^0,=s,ps,t0,c3,+s,(2s,us,)20,*30,gs,hs
 * vim:ts=8
 * vim:sw=8
 * vim:noexpandtab
 */

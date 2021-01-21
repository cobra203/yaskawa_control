#include <cobra_cmd.h>
#include <cobra_console.h>

#if (LOG_CMD_LEVEL > LOG_LEVEL_NOT)
#define CMD_INFO(fm, ...) { \
		console_cmdline_clean(); \
		console("CMD      : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define CMD_INFO(fm, ...)
#endif

#if (LOG_CMD_LEVEL > LOG_LEVEL_INFO)
#define CMD_DEBUG(fm, ...) { \
		console_cmdline_clean(); \
		console("CMD      : " fm, ##__VA_ARGS__); \
		console_cmdline_restore(); \
	}
#else
#define CMD_DEBUG(fm, ...)
#endif

#define CMD_LOG(level, fm, ...) CMD_##level(fm, ##__VA_ARGS__)

CBA_BOOL cobra_cmd_parse(const char *cmdline, COBRA_CMD_S *cmd)
{
#if COBRA_CMD_ENABLE
	uint8_t i = 0, space = 0;
	CBA_BOOL rm_space = CBA_FALSE;
	CBA_BOOL no_subcmd = CBA_FALSE;

	/* 1. parse prefix */
	for(i = 0; i < _PREFIX_SIZE_; i++, cmdline++) {
		if(' ' == *cmdline && CBA_FALSE == rm_space) {
			space++;
			continue;
		}
		if(CBA_FALSE == rm_space) {
			rm_space = CBA_TRUE;
		}
		if(' ' == *cmdline || '_' == *cmdline || '\0' == *cmdline) {
			if(i > 0) {
				cmd->prefix[i - space] = '\0';
				break;
			}
		}
		cmd->prefix[i - space] = *cmdline;
	}
	if(i >= _PREFIX_SIZE_) {
		return CBA_FAILURE;
	}
	if('\0' == *cmdline) {
		cmd->subcmd[0] = '\0';
		cmd->arg[0] = '\0';
		return CBA_SUCCESS;
	}
	if(' ' == *cmdline++) {
		no_subcmd = CBA_TRUE;
		cmd->subcmd[0] = '\0';
	}

	if(CBA_FALSE == no_subcmd) {
		/* 2. parse subcmd */
		for(i = 0; i < _SUBCMD_SIZE_; i++, cmdline++) {
			if(' ' == *cmdline || '\0' == *cmdline) {
				cmd->subcmd[i] = '\0';
				break;
			}
			cmd->subcmd[i] = *cmdline;
		}
		if(i >= _SUBCMD_SIZE_) {
			return CBA_FAILURE;
		}
		if('\0' == *cmdline++) {
			cmd->arg[0] = '\0';
			return CBA_SUCCESS;
		}
	}

	/* 3. parse arg */
	for(; ' ' == *cmdline; cmdline++);
	for(i = 0; i < _ARG_SIZE_; i++, cmdline++) {
		if('\0' == *cmdline) {
			cmd->arg[i] = '\0';
			break;
		}
		cmd->arg[i] = *cmdline;
	}
	if(i >= _ARG_SIZE_) {
		return CBA_FAILURE;
	}

#endif /* COBRA_CMD_ENABLE */
	return CBA_SUCCESS;
}

CBA_BOOL cobra_cmd_process(COBRA_CMD_S *head, COBRA_CMD_S *cmd)
{
#if COBRA_CMD_ENABLE
	COBRA_CMD_S *pos;

	if(!head->list.next || !head->list.prev) {
		return CBA_FAILURE;
	}

	list_for_each_entry(pos, &head->list, COBRA_CMD_S, list) {
		if(!strcmp(cmd->prefix, pos->prefix) &&
			!strcmp(cmd->subcmd, pos->subcmd)) {
			memcpy(pos->arg, cmd->arg, strlen(cmd->arg) + 1);

			pos->status = cmd->status;
			if(strlen(cmd->subcmd)) {
				CMD_LOG(DEBUG, "%s_%s: process start status[%d]\r\n", pos->prefix, pos->subcmd, pos->status);
			}
			else {
				CMD_LOG(DEBUG, "%s: process start status[%d]\r\n", pos->prefix, pos->status);
			}
			pos->process(pos);
			cmd->status = pos->status;

			if(strlen(cmd->subcmd)) {
				CMD_LOG(DEBUG, "%s_%s: process end status[%d]\r\n", pos->prefix, pos->subcmd, pos->status);
			}
			else {
				CMD_LOG(DEBUG, "%s: process end status[%d]\r\n", pos->prefix, pos->status);
			}
			return CBA_SUCCESS;
		}
	}

#endif /* COBRA_CMD_ENABLE */
	return CBA_FAILURE;
}

CBA_BOOL cobra_cmd_register(COBRA_CMD_S *head, COBRA_CMD_S *cmd)
{
#if COBRA_CMD_ENABLE
	if(head->list.next && head->list.prev) {
		list_add_tail(&cmd->list, &head->list);
		return CBA_SUCCESS;
	}
	else {
		CMD_LOG(INFO, "%s Invalid Shortcut Head\n", __func__);
	}
#endif /* COBRA_CMD_ENABLE */
	return CBA_FAILURE;
}

void cobra_cmd_init(COBRA_CMD_S *head)
{
#if COBRA_CMD_ENABLE
	INIT_LIST_HEAD(&head->list);

	CMD_LOG(INFO, "%s ... OK\r\n", __func__);
#endif /* COBRA_CMD_ENABLE */
}

#include "minishell/minishell.h"

char	*expand_variable(char *cursor, int j, char **envp, int exit_code);
int	key_length(char *start);

void	iterate_ast(t_minishell *ms, t_ast_node *node, int level)
{
	if (node == NULL) return;
	// for (int i = 0; i < level; ++i) printf("  ");
	// printf("%d: %s\n", node->type, node->data ? node->data : "NULL");
	if (node->type == N_CMD_WORD)
		node->data = expand(ms, node->data);
	// printf("fd: %d\n", node->fd);
	// printf("is_heredoc: %d\n", node->is_heredoc);
	if (node->child)
		iterate_ast(ms, node->child, level + 2);
	if (node->sibling)
		iterate_ast(ms, node->sibling, level);
}

char *expand(t_minishell *ms, char *cmds)
{
    int i;
    char *expanded;
    char *tmp;
	bool	is_quoted;

	is_quoted = false;
    i = -1;
    while (cmds[++i])
    {
		if (cmds[i] == '\'')
			is_quoted = !is_quoted;
		if (cmds[i] == '$' && !is_quoted)
		{
			expanded = expand_variable(cmds, i, ms->envp, ms->exit_code);
			tmp = expanded;
			expanded = ft_strjoin(expanded, cmds + i + 1 + key_length(cmds + i + 1));
			free(tmp);
			free(cmds);
			cmds = expanded;
		}
	}
	printf("expanded: %s\n", cmds);
	return (cmds);
}

int	key_length(char *start)
{
	char *key;
	int len;

	key = get_env_key(start);
	len = ft_strlen(key);
	free(key);
	return (len);
}

char	*expand_variable(char *cursor, int j, char **envp, int exit_code)
{
	char *expanded;
	char *tmp;
	char *value;
	char *key;

	tmp = ft_substr(cursor, 0, j);
	if (cursor[j+1] && cursor[j+1] == '?')
		expanded = ft_strjoin(tmp, ft_itoa(exit_code));
	else
	{
		key = get_env_key(cursor + j + 1);
		value = get_env_value(key, envp);
		expanded = ft_strjoin(tmp, value);
		free(value);
		free(key);
	}
	free(tmp);
	return (expanded);
}

bool	is_between_single_quotes(char *str, int i)
{
	int count;

	count = 0;
	while (--i >= 0)
	{
		if (str[i] == '\'')
			count++;
	}
	return (count % 2);
}

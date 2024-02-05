#include "minishell/execution/executor.h"
#include "minishell/execution/builtins.h"
#include "minishell/minishell.h"
#include "minishell/error.h"

int	exec_cmd(char **cmds, char *cmd, char ***envp);
// TODO: change arguments to be the expected structs
void	executor(char **cmds, char ***envp, int out_fd, int in_fd)
{
	int		pipe_io[2];
	int		system_io[2];
	pid_t	pid;
	int		i;
	// int		status;

	i = 0;
	system_io[0] = dup(STDIN_FILENO);
	system_io[1] = dup(STDOUT_FILENO);
	while (cmds[i])
	{
		// do input redirection
		do_input_redirection(pipe_io, !i, in_fd);
		// do pipe
		pipe(pipe_io);
		// check if builtin runs on parent
		// if builtin runs on parent, continue without forking
		if (is_builtin(cmds[i]) && runs_on_parent(cmds[i]))
		{
			g_exit_code = exec_builtin(cmds, cmds[i], envp);
			i++;
			// TODO: remove this debug print
			int j = 0;
			while ((*envp)[j])
			{
				printf("envp[%d]: %s\n", j, (*envp)[j]);
				j++;
			}
			// end of debug print
			continue ;
		}
		// else do fork
		pid = fork();
		// do redirection
		// check pid = 0
		// call exec_cmd
		if (pid == 0)
		{
			do_output_redirection(pipe_io, !cmds[i + 1], system_io[1], out_fd);
			use_child_signals();
			exec_cmd(cmds, cmds[i], envp);	
		}
		i++;
	}
	// wait for all children
	// restore io
	restore_io(system_io, pipe_io);
	wait_for_children(pid, cmds);
}

// TODO: change arguments to be the expected structs
int	exec_cmd(char **cmds, char *cmd, char ***envp)
{
	char	*path;
	char	**parts;

	if (is_builtin(cmd))
		return (exec_builtin(cmds, cmd, envp));
	// TODO: check if builtin
	parts = ft_split(cmd, ' ');
	path = get_path(parts[0], *envp);
	if (execve(path, parts, *envp) == -1)
	{
		// TODO: remove perror line
		perror("execve");
		free(path);
		free(parts);
		if (!access(path, F_OK) && access(path, X_OK) < 0)
			err(cmd, "Permission denied", 126);
		err(cmd, "command not found", 127);
	}
	return (EXIT_SUCCESS);
}

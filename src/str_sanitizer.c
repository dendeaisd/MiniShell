#include "minishell/minishell.h"

char	*trim_start(char *str)
{
	char *trimmed;
	int i;
	int j;
	i = 0;
	while(str[i] == ' ' || str[i] == '\t')
		i++;
	trimmed = malloc(ft_strlen(str) - i + 1);
	if (!trimmed)
		return (NULL);
	j = 0;
	while(str[i])
	{
		trimmed[j] = str[i];
		i++;
		j++;
	}
	trimmed[j] = '\0';
	return (trimmed);
}

char	*trim_end(char *str)
{
	char *trimmed;
	int i;
	int j;
	i = 0;
	while(str[i])
		i++;
	i--;
	while(str[i] == ' ' || str[i] == '\t')
		i--;
	trimmed = malloc(i + 2);
	if (!trimmed)
		return (NULL);
	j = 0;
	while(j <= i)
	{
		trimmed[j] = str[j];
		j++;
	}
	trimmed[j] = '\0';
	return (trimmed);
}

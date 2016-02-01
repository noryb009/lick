/**
 * @file
 * @brief An assortment of system-level functions.
 */

#pragma once

/**
 * @brief run a command, similar to system()
 * @param c the command to run
 * @return 1 if the program finished correctly
 */
int run_system(const char *c);
/**
 * @brief run a command, similar to system(), collecting the program output
 * @param[in] c the command to run
 * @param[out] out the standard output and standard error output of the
 *   program, if it finished correctly. Must be freed using free()
 * @return 1 if the program finished correctly
 */
int run_system_output(const char *c, char **out);
/**
 * @brief run a command, similar to system(), returning pipes to the standard
 * input and output of the process
 * @param[in] c the command to run
 * @param[out] input a pointer to a handle to hold the input pipe
 * @param[out] output a pointer to a handle to hold the output pipe
 * @return 1 if the program starts correctly
 */
int run_unprivileged(const char *c, void *input, void *output);
/**
 * @brief run a command, similar to system(), with administrator privileges
 * @param[in] c the command to run
 * @param[in] p the parameters
 * @param[out] ret the return code from the process
 * @return whether the process started correctly
 */
int run_privileged(const char *c, const char *p, int *ret);


/**
 * @brief find the command line of this program
 * @return a string. Do not pass this to free
 */
const char *get_command_line();
/**
 * @brief get the path of the Windows folder
 * @return
 *   the path to the Windows folder. Must be freed using free
 */
char *get_windows_path();
/**
 * @brief get the path of the drive Windows is installed on
 * @return
 *   the drive structure corresponding to the drive Windows is installed on.
 *   must be freed using #free
 */
char *get_windows_drive_path();
/**
 * @brief find the path of this program
 * @return a string. Must be freed using free
 */
char *get_program_path();
/**
 * @brief find the path of this ProgramData folder
 *
 * On Windows 95 - ME, and Vista+, this will be similar to
 * 'C:/ProgramData'. On other systems, it will be similar to
 * 'C:/Documets and Settings/All Users/Application Data'
 *
 * @return a string. Must be freed using free
 */
char *get_config_path();

/** @file cmdline.h
 *  @brief The header file for the command line option parser
 *  generated by GNU Gengetopt version 2.22.6
 *  http://www.gnu.org/software/gengetopt.
 *  DO NOT modify this file, since it can be overwritten
 *  @author GNU Gengetopt by Lorenzo Bettini */

#ifndef CMDLINE_H
#define CMDLINE_H

/* If we use autoconf.  */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h> /* for FILE */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef CMDLINE_PARSER_PACKAGE
/** @brief the program name (used for printing errors) */
#define CMDLINE_PARSER_PACKAGE "climax"
#endif

#ifndef CMDLINE_PARSER_PACKAGE_NAME
/** @brief the complete program name (used for help and version) */
#define CMDLINE_PARSER_PACKAGE_NAME "climax"
#endif

#ifndef CMDLINE_PARSER_VERSION
/** @brief the program version */
#define CMDLINE_PARSER_VERSION "4.1.0"
#endif

/** @brief Where the command line options are stored */
struct gengetopt_args_info
{
  const char *help_help; /**< @brief Print help and exit help description.  */
  const char *full_help_help; /**< @brief Print help, including hidden options, and exit help description.  */
  const char *version_help; /**< @brief Print version and exit help description.  */
  const char *start_help; /**< @brief device power up and start help description.  */
  const char *stop_help; /**< @brief device stop and power down help description.  */
  int volume_arg;	/**< @brief set volume step.  */
  char * volume_orig;	/**< @brief set volume step original value given at command line.  */
  const char *volume_help; /**< @brief set volume step help description.  */
  char ** profile_arg;	/**< @brief set the (new) profile for current operation (default='0').  */
  char ** profile_orig;	/**< @brief set the (new) profile for current operation original value given at command line.  */
  unsigned int profile_min; /**< @brief set the (new) profile for current operation's minimum occurreces */
  unsigned int profile_max; /**< @brief set the (new) profile for current operation's maximum occurreces */
  const char *profile_help; /**< @brief set the (new) profile for current operation help description.  */
  char * startplayback_arg;	/**< @brief Start playback of audio file.  */
  char * startplayback_orig;	/**< @brief Start playback of audio file original value given at command line.  */
  const char *startplayback_help; /**< @brief Start playback of audio file help description.  */
  const char *stopplayback_help; /**< @brief Stop playback if active help description.  */
  char * audiopath_arg;	/**< @brief Path of directory that contains the audio files (optional).  */
  char * audiopath_orig;	/**< @brief Path of directory that contains the audio files (optional) original value given at command line.  */
  const char *audiopath_help; /**< @brief Path of directory that contains the audio files (optional) help description.  */
  char * device_arg;	/**< @brief target name for the interface: i2c, serial, socket, i2c dummy.  */
  char * device_orig;	/**< @brief target name for the interface: i2c, serial, socket, i2c dummy original value given at command line.  */
  const char *device_help; /**< @brief target name for the interface: i2c, serial, socket, i2c dummy help description.  */
  const char *resetMtpEx_help; /**< @brief reset MtpEx register to do re-calibration help description.  */
  const char *reset_help; /**< @brief initialize I2C registers and set ACS. After the command, the state is same power on reset. help description.  */
  char * calibrate_arg;	/**< @brief do calibration with loaded speaker file, --output returns updated speaker file (default='always').  */
  char * calibrate_orig;	/**< @brief do calibration with loaded speaker file, --output returns updated speaker file original value given at command line.  */
  const char *calibrate_help; /**< @brief do calibration with loaded speaker file, --output returns updated speaker file help description.  */
  const char *calshow_help; /**< @brief show calibration impedance value help description.  */
  char ** params_arg;	/**< @brief write the params file directly to the device; depending on header type: patch, speaker, preset, config, drc, eq .  */
  char ** params_orig;	/**< @brief write the params file directly to the device; depending on header type: patch, speaker, preset, config, drc, eq  original value given at command line.  */
  unsigned int params_min; /**< @brief write the params file directly to the device; depending on header type: patch, speaker, preset, config, drc, eq 's minimum occurreces */
  unsigned int params_max; /**< @brief write the params file directly to the device; depending on header type: patch, speaker, preset, config, drc, eq 's maximum occurreces */
  const char *params_help; /**< @brief write the params file directly to the device; depending on header type: patch, speaker, preset, config, drc, eq  help description.  */
  float re0_arg;	/**< @brief set specified re0 or read the current re0.  */
  char * re0_orig;	/**< @brief set specified re0 or read the current re0 original value given at command line.  */
  const char *re0_help; /**< @brief set specified re0 or read the current re0 help description.  */
  char * save_arg;	/**< @brief write settings to binary file without header. the file type extension must be specified as .eq.bin, .speaker.bin, etc.  */
  char * save_orig;	/**< @brief write settings to binary file without header. the file type extension must be specified as .eq.bin, .speaker.bin, etc original value given at command line.  */
  const char *save_help; /**< @brief write settings to binary file without header. the file type extension must be specified as .eq.bin, .speaker.bin, etc help description.  */
  int currentprof_arg;	/**< @brief set the currently (loaded) runing profile to force transition to new profile. This options should be used with profile option.  */
  char * currentprof_orig;	/**< @brief set the currently (loaded) runing profile to force transition to new profile. This options should be used with profile option original value given at command line.  */
  const char *currentprof_help; /**< @brief set the currently (loaded) runing profile to force transition to new profile. This options should be used with profile option help description.  */
  const char *versions_help; /**< @brief print versions and chip rev help description.  */
  int* register_arg;	/**< @brief read tfa register, write if extra arg given.  */
  char ** register_orig;	/**< @brief read tfa register, write if extra arg given original value given at command line.  */
  unsigned int register_min; /**< @brief read tfa register, write if extra arg given's minimum occurreces */
  unsigned int register_max; /**< @brief read tfa register, write if extra arg given's maximum occurreces */
  const char *register_help; /**< @brief read tfa register, write if extra arg given help description.  */
  int* regwrite_arg;	/**< @brief write value for register.  */
  char ** regwrite_orig;	/**< @brief write value for register original value given at command line.  */
  unsigned int regwrite_min; /**< @brief write value for register's minimum occurreces */
  unsigned int regwrite_max; /**< @brief write value for register's maximum occurreces */
  const char *regwrite_help; /**< @brief write value for register help description.  */
  int vstepmsg_arg;	/**< @brief write value to select specific message inside the volumestep (default='100').  */
  char * vstepmsg_orig;	/**< @brief write value to select specific message inside the volumestep original value given at command line.  */
  const char *vstepmsg_help; /**< @brief write value to select specific message inside the volumestep help description.  */
  char * dump_arg;	/**< @brief dump all defined registers or container (default='').  */
  char * dump_orig;	/**< @brief dump all defined registers or container original value given at command line.  */
  const char *dump_help; /**< @brief dump all defined registers or container help description.  */
  char * pin_arg;	/**< @brief control devkit signal pin.  */
  char * pin_orig;	/**< @brief control devkit signal pin original value given at command line.  */
  const char *pin_help; /**< @brief control devkit signal pin help description.  */
  char * wait_arg;	/**< @brief timeout value (msec) for pinevent (0=forever).  */
  char * wait_orig;	/**< @brief timeout value (msec) for pinevent (0=forever) original value given at command line.  */
  const char *wait_help; /**< @brief timeout value (msec) for pinevent (0=forever) help description.  */
  int diag_arg;	/**< @brief run all tests, or single if testnr; optional extra argument: [i2s|dsp|sb|pins] (default='0').  */
  char * diag_orig;	/**< @brief run all tests, or single if testnr; optional extra argument: [i2s|dsp|sb|pins] original value given at command line.  */
  const char *diag_help; /**< @brief run all tests, or single if testnr; optional extra argument: [i2s|dsp|sb|pins] help description.  */
  int* xmem_arg;	/**< @brief access (read/write) tfa xmem.  */
  char ** xmem_orig;	/**< @brief access (read/write) tfa xmem original value given at command line.  */
  unsigned int xmem_min; /**< @brief access (read/write) tfa xmem's minimum occurreces */
  unsigned int xmem_max; /**< @brief access (read/write) tfa xmem's maximum occurreces */
  const char *xmem_help; /**< @brief access (read/write) tfa xmem help description.  */
  int* ymem_arg;	/**< @brief access (read/write) tfa ymem.  */
  char ** ymem_orig;	/**< @brief access (read/write) tfa ymem original value given at command line.  */
  unsigned int ymem_min; /**< @brief access (read/write) tfa ymem's minimum occurreces */
  unsigned int ymem_max; /**< @brief access (read/write) tfa ymem's maximum occurreces */
  const char *ymem_help; /**< @brief access (read/write) tfa ymem help description.  */
  int* pmem_arg;	/**< @brief access (read/write) tfa pmem.  */
  char ** pmem_orig;	/**< @brief access (read/write) tfa pmem original value given at command line.  */
  unsigned int pmem_min; /**< @brief access (read/write) tfa pmem's minimum occurreces */
  unsigned int pmem_max; /**< @brief access (read/write) tfa pmem's maximum occurreces */
  const char *pmem_help; /**< @brief access (read/write) tfa pmem help description.  */
  int* iomem_arg;	/**< @brief access (read/write) tfa iomem.  */
  char ** iomem_orig;	/**< @brief access (read/write) tfa iomem original value given at command line.  */
  unsigned int iomem_min; /**< @brief access (read/write) tfa iomem's minimum occurreces */
  unsigned int iomem_max; /**< @brief access (read/write) tfa iomem's maximum occurreces */
  const char *iomem_help; /**< @brief access (read/write) tfa iomem help description.  */
  char * speaker_arg;	/**< @brief speaker side select: Left, Right, Both or Disable (default='B').  */
  char * speaker_orig;	/**< @brief speaker side select: Left, Right, Both or Disable original value given at command line.  */
  const char *speaker_help; /**< @brief speaker side select: Left, Right, Both or Disable help description.  */
  char * dumpmodel_arg;	/**< @brief dump current speakermodel impedance=z, excursion=x or MBDrcDynamics (default='z').  */
  char * dumpmodel_orig;	/**< @brief dump current speakermodel impedance=z, excursion=x or MBDrcDynamics original value given at command line.  */
  const char *dumpmodel_help; /**< @brief dump current speakermodel impedance=z, excursion=x or MBDrcDynamics help description.  */
  int record_arg;	/**< @brief record speaker state info via I2C and display (default='55').  */
  char * record_orig;	/**< @brief record speaker state info via I2C and display original value given at command line.  */
  const char *record_help; /**< @brief record speaker state info via I2C and display help description.  */
  int count_arg;	/**< @brief number of read cycles to execute, 0 means forever.  */
  char * count_orig;	/**< @brief number of read cycles to execute, 0 means forever original value given at command line.  */
  const char *count_help; /**< @brief number of read cycles to execute, 0 means forever help description.  */
  char * output_arg;	/**< @brief specify the output file for binary speaker state info records, default=stdout.  */
  char * output_orig;	/**< @brief specify the output file for binary speaker state info records, default=stdout original value given at command line.  */
  const char *output_help; /**< @brief specify the output file for binary speaker state info records, default=stdout help description.  */
  int logger_arg;	/**< @brief start datalogger, recording <count> state info lines and binary Z/Xmodels (default='2').  */
  char * logger_orig;	/**< @brief start datalogger, recording <count> state info lines and binary Z/Xmodels original value given at command line.  */
  const char *logger_help; /**< @brief start datalogger, recording <count> state info lines and binary Z/Xmodels help description.  */
  char * ini2cnt_arg;	/**< @brief Generate the container file from an ini file <this>.ini to <this>.cnt.  */
  char * ini2cnt_orig;	/**< @brief Generate the container file from an ini file <this>.ini to <this>.cnt original value given at command line.  */
  const char *ini2cnt_help; /**< @brief Generate the container file from an ini file <this>.ini to <this>.cnt help description.  */
  char * bin2hdr_arg;	/**< @brief Generate a file with header from input binary file.<type> [customer] [application] [type]. Original file is backed up as file.<type>.old file.  */
  char * bin2hdr_orig;	/**< @brief Generate a file with header from input binary file.<type> [customer] [application] [type]. Original file is backed up as file.<type>.old file original value given at command line.  */
  const char *bin2hdr_help; /**< @brief Generate a file with header from input binary file.<type> [customer] [application] [type]. Original file is backed up as file.<type>.old file help description.  */
  int tfa_arg;	/**< @brief provide the tfa device type (default='1').  */
  char * tfa_orig;	/**< @brief provide the tfa device type original value given at command line.  */
  const char *tfa_help; /**< @brief provide the tfa device type help description.  */
  char * load_arg;	/**< @brief read parameter settings from container file.  */
  char * load_orig;	/**< @brief read parameter settings from container file original value given at command line.  */
  const char *load_help; /**< @brief read parameter settings from container file help description.  */
  char * splitparms_arg;	/**< @brief save parameters of the loaded container file to seperate parameter files.  */
  char * splitparms_orig;	/**< @brief save parameters of the loaded container file to seperate parameter files original value given at command line.  */
  const char *splitparms_help; /**< @brief save parameters of the loaded container file to seperate parameter files help description.  */
  char * msgblob_arg;	/**< @brief save all relevant DSP messages from the loaded container file to a single binary.  */
  char * msgblob_orig;	/**< @brief save all relevant DSP messages from the loaded container file to a single binary original value given at command line.  */
  const char *msgblob_help; /**< @brief save all relevant DSP messages from the loaded container file to a single binary help description.  */
  char * server_arg;	/**< @brief run as server (for Linux only, default=`9887') (default='9887').  */
  char * server_orig;	/**< @brief run as server (for Linux only, default=`9887') original value given at command line.  */
  const char *server_help; /**< @brief run as server (for Linux only, default=`9887') help description.  */
  char * client_arg;	/**< @brief run as client (for Linux only, default=`9887') (default='9887').  */
  char * client_orig;	/**< @brief run as client (for Linux only, default=`9887') original value given at command line.  */
  const char *client_help; /**< @brief run as client (for Linux only, default=`9887') help description.  */
  int slave_arg;	/**< @brief override hardcoded I2C slave address.  */
  char * slave_orig;	/**< @brief override hardcoded I2C slave address original value given at command line.  */
  const char *slave_help; /**< @brief override hardcoded I2C slave address help description.  */
  int loop_arg;	/**< @brief loop the operation [0=forever].  */
  char * loop_orig;	/**< @brief loop the operation [0=forever] original value given at command line.  */
  const char *loop_help; /**< @brief loop the operation [0=forever] help description.  */
  int verbose_arg;	/**< @brief Enable verbose (mask=timing|i2cserver|socket|scribo) (default='2').  */
  char * verbose_orig;	/**< @brief Enable verbose (mask=timing|i2cserver|socket|scribo) original value given at command line.  */
  const char *verbose_help; /**< @brief Enable verbose (mask=timing|i2cserver|socket|scribo) help description.  */
  int monitor_arg;	/**< @brief Show interpreted I2C writes via dummy.  */
  char * monitor_orig;	/**< @brief Show interpreted I2C writes via dummy original value given at command line.  */
  const char *monitor_help; /**< @brief Show interpreted I2C writes via dummy help description.  */
  char * trace_arg;	/**< @brief Enable I2C transaction tracing to stdout/file.  */
  char * trace_orig;	/**< @brief Enable I2C transaction tracing to stdout/file original value given at command line.  */
  const char *trace_help; /**< @brief Enable I2C transaction tracing to stdout/file help description.  */
  const char *rpc_help; /**< @brief Enable DSP communication while the audio processing is running help description.  */
  
  unsigned int help_given ;	/**< @brief Whether help was given.  */
  unsigned int full_help_given ;	/**< @brief Whether full-help was given.  */
  unsigned int version_given ;	/**< @brief Whether version was given.  */
  unsigned int start_given ;	/**< @brief Whether start was given.  */
  unsigned int stop_given ;	/**< @brief Whether stop was given.  */
  unsigned int volume_given ;	/**< @brief Whether volume was given.  */
  unsigned int profile_given ;	/**< @brief Whether profile was given.  */
  unsigned int startplayback_given ;	/**< @brief Whether startplayback was given.  */
  unsigned int stopplayback_given ;	/**< @brief Whether stopplayback was given.  */
  unsigned int audiopath_given ;	/**< @brief Whether audiopath was given.  */
  unsigned int device_given ;	/**< @brief Whether device was given.  */
  unsigned int resetMtpEx_given ;	/**< @brief Whether resetMtpEx was given.  */
  unsigned int reset_given ;	/**< @brief Whether reset was given.  */
  unsigned int calibrate_given ;	/**< @brief Whether calibrate was given.  */
  unsigned int calshow_given ;	/**< @brief Whether calshow was given.  */
  unsigned int params_given ;	/**< @brief Whether params was given.  */
  unsigned int re0_given ;	/**< @brief Whether re0 was given.  */
  unsigned int save_given ;	/**< @brief Whether save was given.  */
  unsigned int currentprof_given ;	/**< @brief Whether currentprof was given.  */
  unsigned int versions_given ;	/**< @brief Whether versions was given.  */
  unsigned int register_given ;	/**< @brief Whether register was given.  */
  unsigned int regwrite_given ;	/**< @brief Whether regwrite was given.  */
  unsigned int vstepmsg_given ;	/**< @brief Whether vstepmsg was given.  */
  unsigned int dump_given ;	/**< @brief Whether dump was given.  */
  unsigned int pin_given ;	/**< @brief Whether pin was given.  */
  unsigned int wait_given ;	/**< @brief Whether wait was given.  */
  unsigned int diag_given ;	/**< @brief Whether diag was given.  */
  unsigned int xmem_given ;	/**< @brief Whether xmem was given.  */
  unsigned int ymem_given ;	/**< @brief Whether ymem was given.  */
  unsigned int pmem_given ;	/**< @brief Whether pmem was given.  */
  unsigned int iomem_given ;	/**< @brief Whether iomem was given.  */
  unsigned int speaker_given ;	/**< @brief Whether speaker was given.  */
  unsigned int dumpmodel_given ;	/**< @brief Whether dumpmodel was given.  */
  unsigned int record_given ;	/**< @brief Whether record was given.  */
  unsigned int count_given ;	/**< @brief Whether count was given.  */
  unsigned int output_given ;	/**< @brief Whether output was given.  */
  unsigned int logger_given ;	/**< @brief Whether logger was given.  */
  unsigned int ini2cnt_given ;	/**< @brief Whether ini2cnt was given.  */
  unsigned int bin2hdr_given ;	/**< @brief Whether bin2hdr was given.  */
  unsigned int tfa_given ;	/**< @brief Whether tfa was given.  */
  unsigned int load_given ;	/**< @brief Whether load was given.  */
  unsigned int splitparms_given ;	/**< @brief Whether splitparms was given.  */
  unsigned int msgblob_given ;	/**< @brief Whether msgblob was given.  */
  unsigned int server_given ;	/**< @brief Whether server was given.  */
  unsigned int client_given ;	/**< @brief Whether client was given.  */
  unsigned int slave_given ;	/**< @brief Whether slave was given.  */
  unsigned int loop_given ;	/**< @brief Whether loop was given.  */
  unsigned int verbose_given ;	/**< @brief Whether verbose was given.  */
  unsigned int monitor_given ;	/**< @brief Whether monitor was given.  */
  unsigned int trace_given ;	/**< @brief Whether trace was given.  */
  unsigned int rpc_given ;	/**< @brief Whether rpc was given.  */

} ;

/** @brief The additional parameters to pass to parser functions */
struct cmdline_parser_params
{
  int override; /**< @brief whether to override possibly already present options (default 0) */
  int initialize; /**< @brief whether to initialize the option structure gengetopt_args_info (default 1) */
  int check_required; /**< @brief whether to check that all required options were provided (default 1) */
  int check_ambiguity; /**< @brief whether to check for options already specified in the option structure gengetopt_args_info (default 0) */
  int print_errors; /**< @brief whether getopt_long should print an error message for a bad option (default 1) */
} ;

/** @brief the purpose string of the program */
extern const char *gengetopt_args_info_purpose;
/** @brief the usage string of the program */
extern const char *gengetopt_args_info_usage;
/** @brief the description string of the program */
extern const char *gengetopt_args_info_description;
/** @brief all the lines making the help output */
extern const char *gengetopt_args_info_help[];
/** @brief all the lines making the full help output (including hidden options) */
extern const char *gengetopt_args_info_full_help[];

/**
 * The command line parser
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser (int argc, char **argv,
  struct gengetopt_args_info *args_info);

/**
 * The command line parser (version with additional parameters - deprecated)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param override whether to override possibly already present options
 * @param initialize whether to initialize the option structure my_args_info
 * @param check_required whether to check that all required options were provided
 * @return 0 if everything went fine, NON 0 if an error took place
 * @deprecated use cmdline_parser_ext() instead
 */
int cmdline_parser2 (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  int override, int initialize, int check_required);

/**
 * The command line parser (version with additional parameters)
 * @param argc the number of command line options
 * @param argv the command line options
 * @param args_info the structure where option information will be stored
 * @param params additional parameters for the parser
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_ext (int argc, char **argv,
  struct gengetopt_args_info *args_info,
  struct cmdline_parser_params *params);

/**
 * Save the contents of the option struct into an already open FILE stream.
 * @param outfile the stream where to dump options
 * @param args_info the option struct to dump
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_dump(FILE *outfile,
  struct gengetopt_args_info *args_info);

/**
 * Save the contents of the option struct into a (text) file.
 * This file can be read by the config file parser (if generated by gengetopt)
 * @param filename the file where to save
 * @param args_info the option struct to save
 * @return 0 if everything went fine, NON 0 if an error took place
 */
int cmdline_parser_file_save(const char *filename,
  struct gengetopt_args_info *args_info);

/**
 * Print the help
 */
void cmdline_parser_print_help(void);
/**
 * Print the full help (including hidden options)
 */
void cmdline_parser_print_full_help(void);
/**
 * Print the version
 */
void cmdline_parser_print_version(void);

/**
 * Initializes all the fields a cmdline_parser_params structure 
 * to their default values
 * @param params the structure to initialize
 */
void cmdline_parser_params_init(struct cmdline_parser_params *params);

/**
 * Allocates dynamically a cmdline_parser_params structure and initializes
 * all its fields to their default values
 * @return the created and initialized cmdline_parser_params structure
 */
struct cmdline_parser_params *cmdline_parser_params_create(void);

/**
 * Initializes the passed gengetopt_args_info structure's fields
 * (also set default values for options that have a default)
 * @param args_info the structure to initialize
 */
void cmdline_parser_init (struct gengetopt_args_info *args_info);
/**
 * Deallocates the string fields of the gengetopt_args_info structure
 * (but does not deallocate the structure itself)
 * @param args_info the structure to deallocate
 */
void cmdline_parser_free (struct gengetopt_args_info *args_info);

/**
 * Checks that all the required options were specified
 * @param args_info the structure to check
 * @param prog_name the name of the program that will be used to print
 *   possible errors
 * @return
 */
int cmdline_parser_required (struct gengetopt_args_info *args_info,
  const char *prog_name);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* CMDLINE_H */

/**
 * @file clap_action.h
 * @brief Action types and handlers for libclap
 */

#ifndef CLAP_ACTION_H
#define CLAP_ACTION_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Action types for argument processing
 */
typedef enum {
    CLAP_ACTION_STORE,          /**< Store the value (default) */
    CLAP_ACTION_STORE_CONST,    /**< Store a constant value */
    CLAP_ACTION_STORE_TRUE,     /**< Store true (boolean flag) */
    CLAP_ACTION_STORE_FALSE,    /**< Store false (boolean flag) */
    CLAP_ACTION_APPEND,         /**< Append to a list */
    CLAP_ACTION_APPEND_CONST,   /**< Append constant to a list */
    CLAP_ACTION_COUNT,          /**< Count occurrences */
    CLAP_ACTION_HELP,           /**< Display help and exit */
    CLAP_ACTION_VERSION,        /**< Display version and exit */
    CLAP_ACTION_CUSTOM          /**< User-defined action */
} clap_action_t;

#ifdef __cplusplus
}
#endif

#endif /* CLAP_ACTION_H */

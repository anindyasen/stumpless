/* SPDX-License-Identifier: Apache-2.0 */

/*
 * Copyright 2018-2021 Joel E. Anderson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @dir target
 * Header files for target types provided by the library.
 */

/** @file
 * General types and functions for working with all targets.
 */

#ifndef __STUMPLESS_TARGET_H
#  define __STUMPLESS_TARGET_H

#  include <stdarg.h>
#  include <stddef.h>
#  include <stumpless/config.h>
#  include <stumpless/entry.h>
#  include <stumpless/id.h>

/** The file opened if the default target is to a file. */
#  define STUMPLESS_DEFAULT_FILE "stumpless-default.log"

/** The name of the default target. */
#  define STUMPLESS_DEFAULT_TARGET_NAME "stumpless-default"

#  ifdef __cplusplus
extern "C" {
#  endif

/** Types of targets that may be created. */
enum stumpless_target_type {
  STUMPLESS_BUFFER_TARGET, /**< write to a character buffer */
  STUMPLESS_FILE_TARGET, /**< write to a file */
  STUMPLESS_FUNCTION_TARGET, /**< call a custom function */
  STUMPLESS_JOURNALD_TARGET, /**< send to the systemd journald service */
  STUMPLESS_NETWORK_TARGET, /**< send to a network endpoint */
  STUMPLESS_SOCKET_TARGET, /**< write to a Unix socket */
  STUMPLESS_STREAM_TARGET, /**< write to a FILE stream */
  STUMPLESS_WINDOWS_EVENT_LOG_TARGET /**< add to the Windows Event Log */
};

/**
 * A target that log entries can be sent to.
 */
struct stumpless_target {
/** A unique identifier of this target. */
  stumpless_id_t id;
/**
 * The type of this target. The type of a target will not change over the
 * lifetime of the target.
 */
  enum stumpless_target_type type;
/**
 * The name of this target.
 *
 * For some target types, the name may have more significcance than a simple
 * identifier. For example, the name of a file target will be the file that the
 * target writes to.
 *
 * The name of the target will be NULL-terminated.
 */
  char *name;
/** The number of characters in the name. */
  size_t name_length;
/** A bitwise or of all options set on the target. */
  int options;
/** The prival used for messages without a severity or facility provided. */
  int default_prival;
/**
 * The app name used for messages without one provided.
 *
 * The default app name will not be NULL-terminated.
 */
  char *default_app_name;
/** The number of characters in the default app name. */
  size_t default_app_name_length;
/**
 * The msgid used for messages without one provided.
 *
 * The default msgid will not be NULL-terminated.
 */
  char *default_msgid;
/** The number of characters in the default msgid. */
  size_t default_msgid_length;
/**
 * The log mask used by the target.
 *
 * This member is currently not used. In the future it may be used in a similar
 * manner to the masks used by \c setlogmask in syslog.h, or it may be removed.
 */
  int mask;
#  ifdef STUMPLESS_THREAD_SAFETY_SUPPORTED
/*
 * In thread-safe builds the memory at the end of the target holds a mutex that
 * is used to coordinate access to the target. However the type info is not
 * included in the struct definition in the public headers as it is
 * configuration-specific and would complicate the public headers significantly
 * if they were to stay portable.
 */
#  endif
};

/**
 * Logs a message to the default target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param ... Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int stump( const char *message, ... );

/**
 * Logs a message to the default target with the given priority.
 *
 * This function can serve as a replacement for the traditional \c syslog
 * function.
 *
 * For detailed information on what the default target will be for a given
 * system, check the stumpless_get_default_target() function documentation.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param priority The priority of the message - this should be the bitwise or
 * of a single STUMPLESS_SEVERITY and single STUMPLESS_FACILITY value.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param ... Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of specifiers
 * given.
 */
void
stumplog( int priority, const char *message, ... );

/**
 * Adds an entry into a given target. This is the primary logging function of
 * stumpless; all other logging functions call this one after performing any
 * setup specific to themselves.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked.
 *
 * @param target The target to send the message to.
 *
 * @param entry The entry to send to the target.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int
stumpless_add_entry( struct stumpless_target *target,
                     const struct stumpless_entry *entry );

/**
 * Adds a log message with a priority to a given target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param target The target to send the message to.
 *
 * @param priority The priority of the message - this should be the bitwise or
 * of a single STUMPLESS_SEVERITY and single STUMPLESS_FACILITY value.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param ... Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int
stumpless_add_log( struct stumpless_target *target,
                   int priority,
                   const char *message,
                   ... );

/**
 * Adds a message to a given target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param target The target to send the message to.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf. This may be NULL, in which case an event with
 * no message is logged.
 *
 * @param ... Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int
stumpless_add_message( struct stumpless_target *target,
                       const char *message,
                       ... );

/**
 * Closes a target.
 *
 * This function can be used when you'd like to avoid checking the type of the
 * target and then calling the appropriate close function. Note that use of this
 * doesn't actually avoid the check - it just does the check on your behalf. It
 * is more efficient to call the specific close function if you know the type of
 * the target.
 *
 * **Thread Safety: MT-Unsafe**
 * This function is not thread safe as it destroys resources that other threads
 * would use if they tried to reference this target.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the destruction
 * of a lock that may be in use as well as the use of the memory deallocation
 * function to release memory.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, as the cleanup of the lock may not be completed, and the memory
 * deallocation function may not be AC-Safe itself.
 *
 * @param target The target to close.
 */
void
stumpless_close_target( struct stumpless_target *target );

/**
 * Gets the current target.
 *
 * The current target is either the last target that was opened, set by a call
 * to stumpless_set_current_target(), or the default target if neither of the
 * former exists.
 *
 * If the target that is designated as the current target is closed, then the
 * current target will be reset to the default target until another target is
 * opened.
 *
 * Be careful not to confuse this target with the default target, which is the
 * target used when no suitable current target exists. While these may be the
 * same in some cases, they will not always be.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Atomic operations are used to work with the
 * default target.
 *
 * **Async Signal Safety: AS-Unsafe heap**
 * This function is not safe to call from signal handlers due to the
 * possible use of memory management functions to create the default target.
 *
 * **Async Cancel Safety: AC-Unsafe heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of memory management functions.
 *
 * @return The current target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_get_current_target( void );

/**
 * Gets the default facility of a target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate changes to the
 * target while it is being read.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate the read of the target.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to get the facility from..
 *
 * @return The default facility if no error is encountered. If an error is
 * encountered, then -1 is returned and an error code is set appropriately.
 */
int
stumpless_get_default_facility( const struct stumpless_target *target );

/**
 * Gets the default target.
 *
 * The default target is opened when a logging call is made with no target
 * open. It will not be opened until either this happens or a call to this
 * function is made. It will not be closed until a call to stumpless_free_all()
 * is made.
 *
 * Be careful not to confuse this target with the current target, which is the
 * last target opened or set via stumpless_set_current_target(). While these
 * will return the same target in some cases, such as if they are called before
 * opening any targets, they are not equivalent.
 *
 * The default target type will change depending on the configuration of the
 * system configuration. If Windows Event Log targets are supported, then the
 * default target will log to an event log named
 * \c STUMPLESS_DEFAULT_TARGET_NAME. If Windows Event Log targets are not
 * supported and socket targets are, then the default target will point at the
 * socket named in STUMPLESS_DEFAULT_SOCKET, which will be /var/run/syslog if
 * it existed at build time, or else /dev/log. If neither of these target types
 * are supported then a file target is opened to log to the file named in
 * \c STUMPLESS_DEFAULT_FILE.
 *
 * The default target will not have any options set, and will have a default
 * facility of \c STUMPLESS_FACILITY_USER. These settings may be modified by
 * calling the appropriate modifiers on the target after retrieving it with this
 * function.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Atomic operations are used to work with the
 * default target.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the
 * possible use of memory management functions to create the default target.
 *
 * **Async Cancel Safety: AC-Unsafe heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of memory management functions.
 *
 * @return The default target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_get_default_target( void );

/**
 * Gets a given option of a target.
 *
 * While the returned value is the option if it is set, code can also simply
 * check the truth value of the return to see if the provided option is set.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate changes to the
 * target while it is being read.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate the read of the target.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to get the option from.
 *
 * @param option The option to check the target for.
 *
 * @return The option if it is set on the target. If the option is not set,
 * then zero is returned. If an error is encountered, then zero is returned
 * and an error code is set appropriately.
 */
int
stumpless_get_option( const struct stumpless_target *target, int option );

/**
 * Returns the default app name of the given target. The character buffer must
 * be freed by the caller when it is no longer needed to avoid memory leaks.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate the read of the
 * target with other accesses and modifications.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate access and the use of memory management
 * functions to create the result.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked as well as
 * memory management functions.
 *
 * @since release v2.0.0
 *
 * @param target The target to get the app name from.
 *
 * @return The default app name of the target, if no error is encountered. If an
 * error is encountered, then NULL is returned and an error code is set
 * appropriately.
 */
const char *
stumpless_get_target_default_app_name( const struct stumpless_target *target );

/**
 * Returns the default msgid of the given target. The character buffer must be
 * freed by the caller when it is no longer needed to avoid memory leaks.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate the read of the
 * target with other accesses and modifications.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate access and the use of memory management
 * functions to create the result.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked as well as
 * memory management functions.
 *
 * @since release v2.0.0
 *
 * @param target The target to get the msgid from.
 *
 * @return The default msgid of the target, if no error is encountered. If an
 * error is encountered, then NULL is returned and an error code is set
 * appropriately.
 */
const char *
stumpless_get_target_default_msgid( const struct stumpless_target *target );

/**
 * Returns the name of the given target. The character buffer must be freed by
 * the caller when it is no longer needed to avoid memory leaks.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate the read of the
 * target with other accesses and modifications.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate access and the use of memory management
 * functions to create the result.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked as well as
 * memory management functions.
 *
 * @since release v2.0.0
 *
 * @param target The target to get the name from.
 *
 * @return The name of target, if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
const char *
stumpless_get_target_name( const struct stumpless_target *target );

/**
 * Opens a target that has already been created and configured.
 *
 * Targets that have been created using the \c stumpless_new_*_target family of
 * functions need to be opened once they have been configured with all of the
 * desired parameters, or if a previous change caused them to pause.
 *
 * If the provided target has not had all mandatory settings configured or some
 * other error is encountered, then the operation will fail and the target will
 * remain in a paused state.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate accesses and
 * updates to the current target.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate access.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to open.
 *
 * @return The opened target if it was opened successfully (which will be
 * equal to the target argument). If an error was encountered, then NULL is
 * returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_open_target( struct stumpless_target *target );

/**
 * Sets the target used when one is not provided.
 *
 * Without being set, the current target will be the last one opened, or the
 * default target if a target has not yet been opened. The current target is
 * used by functions like stumplog() and stumpless() where a target is not
 * explicitly provided to the call.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Atomic operations are used to work with the
 * default target.
 *
 * **Async Signal Safety: AS-Safe**
 * This function is safe to call from signal handlers as it only consists of
 * an atomic read.
 *
 * **Async Cancel Safety: AC-Safe**
 * This function is safe to call from threads that may be asynchronously
 * cancelled, as it only consists of an atomic read.
 *
 * @param target The target to use as the current target.
 */
void
stumpless_set_current_target( struct stumpless_target *target );

/**
 * Sets the default facility of a target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate changes to the
 * target while it is being modified.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate changes.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to modify.
 *
 * @param default_facility The default facility to use on the target. This
 * should be a STUMPLESS_FACILITY value.
 *
 * @return The modified target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_set_default_facility( struct stumpless_target *target,
                                int default_facility );

/**
 * Sets an option on a target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate changes to the
 * target while it is being modified.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate changes.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to modify.
 *
 * @param option The option to set on the target. This should be a
 * STUMPLESS_OPTION value.
 *
 * @return The modified target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_set_option( struct stumpless_target *target, int option );

/**
 * Sets the default app name for a given target.
 *
 * **Thread Safety: MT-Safe race:app_name**
 * This function is thread safe, of course assuming that the name is not changed
 * by any other threads during exeuction. A mutex is used to coordinate changes
 * to the target while it is being modified.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate changes and the use of memory management
 * functions to create the new name and free the old one.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked as well as
 * memory management functions.
 *
 * @param target The target to modify.
 *
 * @param app_name The new default app name, as a NULL-terminated string.
 * The app name length is restricted to be 48 characters or less.
 *
 * @return The modified target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_set_target_default_app_name( struct stumpless_target *target,
                                       const char *app_name );

/**
 * Sets the default msgid for a given target.
 *
 * **Thread Safety: MT-Safe race:msgid**
 * This function is thread safe, of course assuming that the msgid is not
 * changed by any other threads during exeuction. A mutex is used to coordinate
 * changes to the target while it is being modified.
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate changes and the use of memory management
 * functions to create the new name and free the old one.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked as well as
 * memory management functions.
 *
 * @param target The target to modify.
 *
 * @param msgid The new default msgid, as a NULL-terminated string. The string 
 * must be in the ASCII printable range 33 <= character <= 126 as specified in 
 * RFC5424.
 *
 * @return The modified target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_set_target_default_msgid( struct stumpless_target *target,
                                    const char *msgid );

/**
 * Checks to see if the given target is open.
 *
 * For targets that are opened with a single \c open function call, they will
 * likely be considered open as long as they are non-NULL, as the target
 * creation would otherwise fail before it could be opened. Targets are
 * be considered in a paused state if they were created but not opened yet, for
 * example with \c stumpless_new_network_target as opposed to
 * \c stumpless_open_network_target. Targets may also be paused if a settings
 * change has been made that could not be validated, such as changing the port
 * on a TCP network target to one that does not respond on the server.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate changes to the
 * target while it is being accessed.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate changes.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to check.
 *
 * @return The target if it is currently open, and NULL if not.
 */
const struct stumpless_target *
stumpless_target_is_open( const struct stumpless_target *target );

/**
 * Unsets an option on a target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. A mutex is used to coordinate changes to the
 * target while it is being modified.
 *
 * **Async Signal Safety: AS-Unsafe lock**
 * This function is not safe to call from signal handlers due to the use of a
 * non-reentrant lock to coordinate changes.
 *
 * **Async Cancel Safety: AC-Unsafe lock**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of a lock that could be left locked.
 *
 * @param target The target to modify.
 *
 * @param option The option to unset on the target. This should be a
 * STUMPLESS_OPTION value.
 *
 * @return The modified target if no error is encountered. If an error is
 * encountered, then NULL is returned and an error code is set appropriately.
 */
struct stumpless_target *
stumpless_unset_option( struct stumpless_target *target, int option );

/**
 * Logs a message to the default target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param subs Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given. This list must be started via \c va_start before being
 * used, and \c va_end should be called afterwards, as this function does not
 * call it.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int
vstump( const char *message, va_list subs );

/**
 * Logs a message to the default target with the given priority. Can serve as
 * a replacement for the traditional \c vsyslog function.
 *
 * For detailed information on what the default target will be for a given
 * system, check the stumpless_get_default_target() function documentation.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param priority The priority of the message - this should be the bitwise or
 * of a single STUMPLESS_SEVERITY and single STUMPLESS_FACILITY value.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param subs Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given. This list must be started via \c va_start before being
 * used, and \c va_end should be called afterwards, as this function does not
 * call it.
 */
void
vstumplog( int priority, const char *message, va_list subs );

/**
 * Adds a log message with a priority to a given target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param target The target to send the message to.
 *
 * @param priority The priority of the message - this should be the bitwise or
 * of a single STUMPLESS_SEVERITY and single STUMPLESS_FACILITY value.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param subs Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given. This list must be started via \c va_start before being
 * used, and \c va_end should be called afterwards, as this function does not
 * call it.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int
vstumpless_add_log( struct stumpless_target *target,
                    int priority,
                    const char *message,
                    va_list subs );

/**
 * Adds a message to a given target.
 *
 * **Thread Safety: MT-Safe**
 * This function is thread safe. Different target types handle thread safety
 * differently, as some require per-target locks and others can rely on system
 * libraries to log safely, but all targets support thread safe logging in some
 * manner. For target-specific information on how thread safety is supported and
 * whether AS or AC safety can be assumed, refer to the documentation for the
 * target's header file (in the `stumpless/target` include folder).
 *
 * **Async Signal Safety: AS-Unsafe lock heap**
 * This function is not safe to call from signal handlers as some targets make
 * use of non-reentrant locks to coordinate access. It also may make memory
 * allocation calls to create internal cached structures, and memory allocation
 * may not be signal safe.
 *
 * **Async Cancel Safety: AC-Unsafe lock heap**
 * This function is not safe to call from threads that may be asynchronously
 * cancelled, due to the use of locks in some targets that could be left locked
 * and the potential for memory allocation.
 *
 * @param target The target to send the message to.
 *
 * @param message The message to log, optionally containing any format
 * specifiers valid in \c printf.
 *
 * @param subs Substitutions for any format specifiers provided in message. The
 * number of substitutions provided must exactly match the number of
 * specifiers given. This list must be started via \c va_start before being
 * used, and \c va_end should be called afterwards, as this function does not
 * call it.
 *
 * @return A non-negative value if no error is encountered. If an error is
 * encountered, then a negative value is returned and an error code is set
 * appropriately.
 */
int
vstumpless_add_message( struct stumpless_target *target,
                        const char *message,
                        va_list subs );

#  ifdef __cplusplus
}                               /* extern "C" */
#  endif

#endif                          /* __STUMPLESS_TARGET_H */

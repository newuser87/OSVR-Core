/** @file
    @brief Header

    Must be c-safe!

    @date 2014

    @author
    Ryan Pavlik
    <ryan@sensics.com>
    <http://sensics.com>

*/

/*
// Copyright 2014 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// 	http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#ifndef INCLUDED_PluginRegistrationC_h_GUID_C019DFA9_5B54_4791_B0A4_040EA20501BA
#define INCLUDED_PluginRegistrationC_h_GUID_C019DFA9_5B54_4791_B0A4_040EA20501BA

/* Internal Includes */
#include <ogvr/PluginKit/Export.h>
#include <ogvr/PluginKit/CommonC.h>

/* Library/third-party includes */
#include <libfunctionality/PluginInterface.h>

/* Standard includes */
#include <stddef.h>

/** @defgroup plugin_registration Plugin Registration
    @brief How to start writing a plugin and advertise your capabilities to the
   core library.
   @ingroup plugin_api

   @{
*/

#ifdef __cplusplus
extern "C" {
#endif

/** @brief This macro begins the entry point function of your plugin.

    Treat it as if it were a function declaration, since that is what it will
   expand to. The function body you write calls some subset of the plugin
   registration methods, then returns either success (OGVR_PLUGIN_SUCCESS)
   or failure (OGVR_PLUGIN_FAILURE).

    Your function body receives a single argument, of type
   OGVR_PluginRegContext,
   named `ctx`. You will need to pass this to registration functions that you
   call.
*/
#define OGVR_PLUGIN(PLUGIN_NAME) LIBFUNC_PLUGIN(PLUGIN_NAME, ctx)

/** @name Hardware Polling
    @brief If your plugin contains drivers for devices that you can detect,
    you'll want to register for hardware polling.
    @{
*/

/** @brief An opaque context pointer passed in to your hardware poll callback,
 * if any.
*/
typedef void *OGVRPluginHardwarePollContext;

/** @brief Function type of a Hardware Poll callback */
typedef OGVR_PluginReturnCode (*OGVRHardwarePollCallback)(
    OGVRPluginHardwarePollContext pollContext, void *userData);

/** @brief Register a callback in your plugin to be notified when hardware
   should be polled again.

   When your callback, a function of type OGVRHardwarePollCallback, is invoked,
   it will receive the same userdata you provide here (if any). Your plugin
   should do whatever probing necessary to detect devices you can handle and
   instantiate the device drivers.

   @param ctx The registration context passed to your entry point.
   @param pollCallback The address of your callback function
   @param userData An optional opaque pointer that will be returned to you when
   the callback you register here is called.
*/
OGVR_PLUGINKIT_EXPORT OGVR_PluginReturnCode
    ogvrPluginRegisterHardwarePollCallback(
        OGVR_PluginRegContext ctx, OGVRHardwarePollCallback pollCallback,
        void *userData OGVR_CPP_ONLY(= NULL));
/** @} */

/** @name Plugin Instance Data
    @brief Plugins "own" the modules instantiated in them. Lifetime must be
    managed appropriately: destroyed on shutdown.

    You can store the instances in any way you would like, as long as you
    register them with appropriate deleter callbacks here.

    @{
*/

/** @brief Function type of a Plugin Data Delete callback */
typedef void (*OGVR_PluginDataDeleteCallback)(void *pluginData);

/** @brief Register plugin data along with an appropriate deleter callback.

    When your callback, a function of type OGVR_PluginDataDeleteCallback, is
   invoked, it will receive the plugin data pointer you provide here. Your
   deleter is responsible for appropriately deleting/freeing/destructing all
   data associated with that pointer.

    This function may be called more than once, to register multiple plugin data
   objects. Callbacks will be called, sorted first by plugin, in reverse order
   of registration.

    @param ctx The registration context passed to your entry point.
    @param deleteCallback The address of your deleter callback function
    @param pluginData A pointer to your data, treated as opaque by this library,
   and passed to your deleter.
*/
OGVR_PLUGINKIT_EXPORT OGVR_PluginReturnCode
    ogvrPluginRegisterDataWithDeleteCallback(
        OGVR_PluginRegContext ctx, OGVR_PluginDataDeleteCallback deleteCallback,
        void *pluginData);
/** @} */

#ifdef __cplusplus
} /* end of extern "C" */
#endif

/** @} */

#endif

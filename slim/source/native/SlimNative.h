#pragma once

/** Basically we need an abstraction over top of the SLIM platform
 *  to allow us to write the glue-logic without having to tamper
 *  with the platform code.  When the platform code makes a call
 *  to a native function, it will lookup up the native function
 *  in a table and call it.  The native function is to be implemented
 *  by the user using the SlimNative interface.  This means that
 *  during SlimVM runtime, SlimNative functions will interact with
 *  the SlimVM through the SlimNative interface.
 */

void slim_native_init() {

}

void slim_native_close() {

}
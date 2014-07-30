Creating Your Own Loader For Mod Loader
==================================================

Mod Loader is fully plugin based, this makes things simpler and more independent, each plugin is responssible for handling a certain kind of file.
   
The interface to communicate with Mod Loader is by using an C event-driven API, however there is a C++ binding, which works better.

We'll only cover the C++ interface on this document.


The C++ interface
-------------------------------

As previosly stated, the communication between the Mod Loader core and the plugin will be, essentially, event-driven. We'll need to create an object implementing the events and then build the plugin as a DLL file.

Please note we shall **NOT** implement a `DllMain` function!


To include the basic C++ interface to interact with Mod Loader, include the header file *modloader/modloader.hpp* which is at the root include directory from the source tree.

The first step is to create an object derived from `modloader::basic_plugin`, we can also copy 'n paste the interface from *src/plugins/template.cpp*.

Then it's necessary to register the existence of the plugin object by using the `REGISTER_ML_PLUGIN(plugin)` macro.

Now it's time to implement the events, they are virtual methods from the `modloader::basic_plugin` object.

### Events

#### GetInfo -- `const info& GetInfo()`

 This method should return the reference to an static `modloader::basic_plugin::info`     object. This info object is defined as:

    struct info
    {
        const char*  name;
        const char*  version;
        const char*  author;
        int          default_priority;
        const char** extable;
    };

 _Where_:
 
  + `name` is unused but should be the name of the plugin
  + `version` is the version of the plugin
  + `author` is the author of the plugin, may be _nullptr_
  + `default_priority` is the plugin events priority in relation with other plugins, use _-1_ for default
  + `extable` is a table of pointers to c-strings specifying the extensions this plugin might handle, the end of the table must be marked by a null pointer. Notice this is merely a hint for faster lookup, extensions that the plugin will receive by the events aren't restricted to those.

#### OnStartup -- [optional] `bool OnStartup()` 

 This event is called when the plugin gets started up, the start up order is undertemined.
 
 The method should return **true** if the startup was successful and **false** otherwise. When the startup wasn't successful the plugin will get unloaded immediatelly without calling `OnShutdown`
 Essentially it should return false if the running game isn't the game the plugin is intended to work with.

#### OnShutdown -- [optional] `bool OnShutdown()` 
 
 This event is called when the plugin gets shutdown.
 
 The method should return **true** if the shutdown was successful and **false** otherwise.
 Currently this return value has no effect, but you shouldn't relly on this behaviour.

#### GetBehaviour -- `int  GetBehaviour(modloader::file& file)`
 
 This event is called to know whether this plugin is responsible for handling the specified `file`.

 The method should return either:
 
  + `MODLOADER_BEHAVIOUR_NO` meaning this file is not handled by this plugin.
  + `MODLOADER_BEHAVIOUR_YES` meaning this file is handled by this plugin, see details below.
  + `MODLOADER_BEHAVIOUR_CALLME` meaning this file is not handled by this plugin but the plugin would like to receive it anyway during the install/reinstall/uninstall process.
 

 When `MODLOADER_BEHAVIOUR_YES` is returned the plugin **shall** set the `modloader::file::behaviour` field of the `file` object.
 This field determines the *behaviour* of the specified file. An behaviour is unique for each unique kind of file.
 That means if two files with the same behaviour are present, only one will get installed (or it'll uninstall the previous one).

 An example is, *a.model* has the same behaviour as another *a.model* but not the same as *b.model*.
 Two files with the same behaviour will never be installed at the same time.
 
 _Special Note_: Since the object is non-const, it's important to note that you shall not write to any field of `file` other than `behaviour` during this event.


#### InstallFile -- `bool   InstallFile(const modloader::file& file)`
 
 This event is called to install a file previosly marked as `MODLOADER_BEHAVIOUR_YES` or `MODLOADER_BEHAVIOUR_CALLME`.
 
 Any previous file with the same behaviour as `file` got uninstalled, essentially calling `UninstallFile` in the process.

 The method should return **true** if the install was successful and **false** otherwise.
 The return value is ignored for *CALLME* handlers.

#### ReinstallFile -- `bool   ReinstallFile(const modloader::file& file)`

 This event is called to reinstall a file previosly installed meaning the file has changed in some way. The file behaviour is guaranted to not have changed during the file change.

 The method should return **true** if the reinstall was successful and **false** otherwise.
 If the install wasn't successful, the file will get uninstalled, essentially by calling `UninstallFile`.
 Please note if both `ReinstallFile` and `UninstallFile` fails, an fatal error will happen!
 The return value is ignored for *CALLME* handlers.

#### UninstallFile -- `bool   UninstallFile(const modloader::file& file)`
 
 This event is called to uninstall a file that was previosly installed.

 The method should return **true** if the uninstall was successful and **false** otherwise.
 The return value is ignored for *CALLME* handlers.

#### Update -- [optional] `void Update()`
 
 This event is called after a serie of *InstallFile / ReinstallFile / UninstallFile* calls to update the state of the plugin if necessary.


### Mod Loader Objects

#### *modloader::basic_plugin*
There are some functions on `modloader::basic_plugin` that may be called to communicate with Mod Loader. There are also some data fields.

+ `loader` stores information about Mod Loader such as the game full path, whether the game has started, etc.
+ `Log(fmt, ...)` and `vLog(fmt, va_list` can be used to log into the logging stream
+ `Error(fmt, ...)` can be used to display a error message box
+ `cast<To>()` can be used to cast a `basic_plugin` object to another derived object.

Not in `modloader::basic_plugin` class, but there is a `modloader::plugin_ptr` that points to your plugin object, as registered in `REGISTER_ML_PLUGIN`

#### *modloader::plugin*
 This object represents an Mod Loader plugin for the loader core.
 Currently this object has no use for plugin creators.
 
#### *modloader::mod*
 This object represents an mod in Mod Loader (*i.e.* one folder at *modloader* directory).
 
#### *modloader::file*
 This object represents an file in Mod Loader, it stores many information about the file, such as it's path, filename hash, size, and more (see *modloader/modloader.hpp* and *modloader/modloader/h*).
 
 It's definitely the most useful object for plugin creators, make sure you understand it before starting your plugin.

### Objects Lifetime 

 The `modloader::file`, `modloader::mod` and `modloader::plugin` objects are guaranted to be valid from the moment it gets passed to `InstallFile` until the file returns from `UninstallFile`. 
 That means you can store it's pointer somewhere at your plugin!

 Notice the object is **NOT** guaranted to be valid after a `GetBehaviour` nor during an `Update` with an object previosly uninstalled.

### Examples
Examples can be found by looking at Mod Loader plugins itself (*src/plugins/*) or by looking at the template for plugins (*src/plugins/template.cpp*)


Utility headers
-------------------------------

Beyond API headers (*include/modloader.h* and *include/modloader.hpp*) there are utility headers on the *include/modloader/util/* directory.
Those utility headers are there to help plugin creators with usual tasks.


There are no licensing issues
-------------------------------

Despite Mod Loader being licensed under GNU General Public License you are completly free to use whatever license you want on your plugin, there's no GPL infection here since you don't link with the infected code and the include files are under the public domain.

However, if you want your plugin to be merged into Mod Loader you shall use at least a GPL-compatible license 

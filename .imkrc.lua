--[[------------------------------------------------------------------------

This file is a configuration file for imk that contains the event handlers.
By default all standard lua libraries are already loaded at this point as well
as an events storage object is created.

The imk will search for this file in the current directory and if not found in
the user's home directory.

Currently the following events are supported:
 - events.open
    triggered when a file is opened. The handler accepts the full path/name of
    the file that triggered the event.
 - events.close
    triggered when a file is closed. The handler accepts the full path/name of
    the file that triggered the event.
 - events.create
    triggered when a file is created in the monitored folder. The handler
    accepts the full path/name of the folder where the file was created.
 - events.read
    triggered when a file is read. The handler accepts the full path/name of
    the file that triggered the event.
 - events.write
    triggered when a file is modified. The handler accepts the full path/name
    of the file that triggered the event.
 - events.delete
    triggered when a file is moved or deleted, or a file was deleted in a
    monitored folder. The handler accepts the full path/name of the file that
    triggered the event.

The following API calls are supported:
    - imk_shell(string) - run a shell command (equivalent to C system(char*)
    - imk_command()     - run default command that is set with -c argument
    - imk_getfiles()    - get an array of all monitored files

--]]-----------------------------------------------------------------------

--------------------------------------
--          event handlers          --
--------------------------------------
function open_cb(fname)
    print("opened " .. fname .. " file")

    --[[
    print "-- monitored files files"
    local files = imk_getfiles()
    for i,v in ipairs(files) do
        print(i .. " : " .. v)
    end

    print "-- monitored files files"
    for i=1,#files do
        print(files[i])
    end
    --]]
end
events.open = open_cb

function close_cb(fname)
    print("closed the " .. fname .. " file")
end
events.close = close_cb

function create_cb(fname)
    print("created the " .. fname .. " file")
end
events.create = create_cb

function read_cb(fname)
    print("accessed the " .. fname .. " file")
    imk_shell("ls -al " .. fname)
end
events.read = read_cb

function write_cb(fname)
    print("modified the " .. fname .. " file")
    imk_command()
end
events.write = write_cb

function delete_cb(fname)
    print("the file " .. fname .. " was deleted")
end
events.delete = delete_cb

----------------------------------------
--          helper functions          --
----------------------------------------

-- returns true if String starts with Start, or false otherwise
function string.starts(String, Start)
   return string.sub(String, 1, string.len(Start))==Start
end

-- returns true if String ends with Start, or false otherwise
function string.ends(String, End)
  return End=='' or string.sub(String, -string.len(End))==End
end

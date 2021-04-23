#! /usr/bin/python2

#################################################
#
# Used to create function tables needed by Omnisquash
#
#################################################

import os

BUILD_DIR = os.path.dirname(os.path.realpath(__file__))
ENTITIES_DIR = "entities/"
ENTITY_TABLES_FILE = "entity_tables.c"
WEAPONS_DIR = "weapons/"
WEAPON_TABLES_FILE = "weapon_tables.c"

ENTITY_INCLUDES = "#include \"entity_tables.h\"\n\n"
ENTITY_TABLES = [
    {
        "read name prefix" : "entity_construct_",
        "write name prefix" : "entity_construct_",
        "read type" : "void ",
        "pointer type" : "entity_construct_ptr_t",
        "table variable" : "entityConstructTable",
    },
    {
        "read name prefix" : "entity_clone_",
        "write name prefix" : "entity_clone_",
        "read type" : "Entity *",
        "pointer type" : "entity_clone_ptr_t",
        "table variable" : "entityCloneTable",
    },
    {
        "read name prefix" : "entity_vtb_",
        "write name prefix" : "&entity_vtb_",
        "read type" : "extern const Entity_vtb",
        "pointer type" : "Entity_vtb *",
        "table variable" : "entityVTBTable",
    }
]
ENTITY_TABLE_SIZE_VARIABLE = "entityTableSize"

WEAPON_INCLUDES = "#include \"weapon_tables.h\"\n\n"
WEAPON_TABLES = [
    {
        "read name prefix" : "weapon_construct_",
        "write name prefix" : "weapon_construct_",
        "read type" : "Weapon *",
        "pointer type" : "weapon_construct_ptr_t",
        "table variable" : "weaponConstructTable"
    }
]
WEAPON_TABLE_SIZE_VARIABLE = "weaponTableSize"


def clean():
    """remove existing table files"""
    try:
        os.remove(ENTITIES_DIR + ENTITY_TABLES_FILE)
        os.remove(WEAPONS_DIR + WEAPON_TABLES_FILE)
    except OSError:
        pass

def findIgnoreList():
    """read the file build.ignore if present and use the list to exclude files
    from the build"""
    toIgnore = []
    try:
        with open(".build.ignore", "r") as fin:
            for line in fin:
                line = line.strip()
                if len(line) > 0 and line[0] != "#":
                    line.replace("/", os.sep)
                    toIgnore.append(BUILD_DIR + line)
    except IOError:
        pass
                
    return toIgnore

def findFiles(toIgnore, folderName):
    """locate c and h files, return lists of header files"""
    files = []
    for (dirpath, dirnames, filenames) in os.walk(BUILD_DIR + "/" + folderName):
        if dirpath in toIgnore:
            continue
    
        for name in filenames:
            if len(name) > 2 and (name[-2:] == ".c" or name[-2:] == ".h"):
                if os.path.join(dirpath, name) not in toIgnore:
                    files.append(name)
    
    #take from list only header files that have a corresponding source file
    headerFiles = []
    for f in files:
        # if f[-2:] == ".h" and f[:-2]+".c" in files:
        if f[-2:] == ".h" and f[-9:] != "_tables.h":
            headerFiles.append(f)
    return sorted(headerFiles)
    
def checkHeaderMethods(headerList, headerFileDirectory, filenamePrefix, signatureList):
    """Check each header file line by line to see if it contains lines of the correct format
    VERY BRITTLE
    
    headerList = list of header filenames, no path
    headerFileDirectory = path to the header files such that we can just add them together
    filenamePrefix = the part of the filename before the ID, used to strip out the ID
    signatureList = list of signatures for special methods to identify
    
    returns a dict of lists indexed the same way as ENTITY_TABLES of booleans for whether or not the given header has the particular method
    """
    result = {}
    
    for h in headerList:
        ID = h[len(filenamePrefix):-2]
        tableEntryIsPresentList = [False] * len(signatureList)
            
        try:
            with open(headerFileDirectory + h, "r") as fin:
                for line in fin:
                    #format the line for easier parsing
                    line = line.strip()
                    line = " ".join(line.split())
                    
                    #check for methods
                    for i in range(0, len(signatureList)):
                        methodName1 = signatureList[i]["read type"] + " " + signatureList[i]["read name prefix"] + ID
                        methodName2 = signatureList[i]["read type"] + signatureList[i]["read name prefix"] + ID
                        
                        if line.startswith(methodName1) or line.startswith(methodName2):
                            tableEntryIsPresentList[i] = True
                        
                result[ID] = tableEntryIsPresentList
                
        except IOError:
            print "Error reading file " + headerFileDirectory + h
            exit(1)
            
    return result

def getTableSize(headerList, prefix):
    """Computes the necessary table size based on ID, because its allowed to skip ID numbers so the number of ID's is not accurate for that"""
    maxId = -1
    
    for h in headerList:
        headerId = int(h[len(prefix):-2])
        if headerId > maxId:
            maxId = headerId
            
    return maxId + 1

def writeConstantsTableFile(fout, headerList, methodDict, includes, signatureList, tableSize, tableSizeVariable):
    """writes out the source file where the tables are filled
    
    fout              = file to write to
    headerList        = files to include, the headers we need for filling the tables
    methodDict        = what gets returned from checkHeaderMethods, a dict of lists of whether or not each ID has the different methods
    includes          = the base include statements to go at the top
    signatureList     = list of method signatures and where to store pointers
    tableSize         = size of table to use, doesn't necessarily match the number of headers in headerList
    tableSizeVariable = string  with name of variable to put table size in
    """
    #first, write the header
    fout.write(includes)
    fout.write("\n")
    for h in headerList:
        fout.write("#include \"" + h + "\"\n")
    fout.write("\n\n")
    
    fout.write("const int " + tableSizeVariable + " = " + str(tableSize) + ";\n")
    fout.write("\n")
    
    for i in range(0, len(signatureList)):
        pointerType = signatureList[i]["pointer type"]
        tableVar = signatureList[i]["table variable"]
        prefix = signatureList[i]["write name prefix"]
    
        fout.write("const " + pointerType + " " + tableVar + "[] = {\n")
        for j in range(0, tableSize):
            ID = str(j).zfill(5)
            tableEntryIsPresentList = methodDict[ID]
            
            if tableEntryIsPresentList[i]:
                fout.write("    " + prefix + ID)
            else:
                fout.write("    NULL")
                
            fout.write(",")
            fout.write("\n")
            
        fout.write("};")
        fout.write("\n")
    
    
#################################################
# Script start
#################################################
#change directory to script location
os.chdir(BUILD_DIR)

#remove the old files - this is important, since this prevents picking up the table files when
#searching for files to put in the table files
clean()

#get the files to not include
toIgnore = findIgnoreList()

#find all header files of each type with matching source file, determine contents
entityHeaders = findFiles(toIgnore, ENTITIES_DIR)
entityHeaderMethods = checkHeaderMethods(entityHeaders, ENTITIES_DIR, "entity_", ENTITY_TABLES)
entityTableSize = getTableSize(entityHeaders, "entity_")

weaponHeaders = findFiles(toIgnore, WEAPONS_DIR)
weaponHeaderMethods = checkHeaderMethods(weaponHeaders, WEAPONS_DIR, "weapon_", WEAPON_TABLES)
weaponTableSize = getTableSize(weaponHeaders, "weapon_")

#write the table files
try:
    with open(ENTITIES_DIR + ENTITY_TABLES_FILE, "w") as fout:
        # writeTableFile(fout, entityHeaders, entityHeaderMethods, ENTITY_INCLUDES, ENTITY_METHOD_HEADER, ENTITY_TABLES, entityTableSize)
        writeConstantsTableFile(fout, entityHeaders, entityHeaderMethods, ENTITY_INCLUDES, ENTITY_TABLES, entityTableSize, ENTITY_TABLE_SIZE_VARIABLE)
except IOError:
    print "Couldn't open " + ENTITY_TABLES_FILE + " for writing"
    exit(1)

try:
    with open(WEAPONS_DIR + WEAPON_TABLES_FILE, "w") as fout:
        writeConstantsTableFile(fout, weaponHeaders, weaponHeaderMethods, WEAPON_INCLUDES, WEAPON_TABLES, weaponTableSize, WEAPON_TABLE_SIZE_VARIABLE)
except IOError:
    print "Couldn't open " + WEAPON_TABLES_FILE + " for writing"
    exit(1)
    

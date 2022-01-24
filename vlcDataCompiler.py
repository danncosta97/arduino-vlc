import os
import json

rootDirectory = 'C:/Users/Daniel/Documents/VLC_DADOS_CLEAN'
# set base strings
with open(f'{rootDirectory}/100BytesString.txt') as string:
    String100Bytes = string.read()
with open(f'{rootDirectory}/500BytesString.txt') as string:
    String500Bytes = string.read()
with open(f'{rootDirectory}/1000BytesString.txt') as string:
    String1000Bytes = string.read()


# Attemp to make os.walk sort numbers
def renameToMilimeters():
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        for subdir in subdirs:
            if(subdir.split('.')[-1] == '0cm'):
                newSubdirName = (f'{subdir.split(".")[0]}0mm')
                os.rename(f'{currentPath}/{subdir}', f'{currentPath}/{newSubdirName}')
            if(subdir.split('.')[-1] == '5cm'):
                newSubdirName = (f'{subdir.split(".")[0]}5mm')
                os.rename(f'{currentPath}/{subdir}', f'{currentPath}/{newSubdirName}')


# Attemp to make os.walk sort numbers
def renameToCentimeters():
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        for subdir in subdirs:
            if(subdir[-3:] == '0mm'):
                newSubdirName = (f'{subdir[:len(subdir)-3]}.0cm')
                print(newSubdirName)
                os.rename(f'{currentPath}/{subdir}', f'{currentPath}/{newSubdirName}')
            if(subdir[-3:] == '5mm'):
                newSubdirName = (f'{subdir[:len(subdir)-3]}.5cm')
                print(newSubdirName)
                os.rename(f'{currentPath}/{subdir}', f'{currentPath}/{newSubdirName}')


def adjustBytesName():
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        for subdir in subdirs:
            if(len(subdir) == 4 and subdir[-1:] == 'B'):
                print(subdir)
                newSubdirName = (f'0{subdir}')
                print(newSubdirName)
                os.rename(f'{currentPath}/{subdir}', f'{currentPath}/{newSubdirName}')


def adjustCentimetersName():
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        for subdir in subdirs:
            if(len(subdir) == 5 and subdir[-2:] == 'cm'):
                print(subdir)
                newSubdirName = (f'0{subdir}')
                print(newSubdirName)
                os.rename(f'{currentPath}/{subdir}', f'{currentPath}/{newSubdirName}')


def adjustFrequencyName():
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        for file in files:
            if(len(file) == 9 and file[-6:] == 'Hz.txt'):
                print(file)
                newFileName = (f'0{file}')
                print(newFileName)
                os.rename(f'{currentPath}/{file}', f'{currentPath}/{newFileName}')


# Generates json for each leave folder, the only that contains files (frequency files) with the trnamission received
# A 'result.json' will be created at rootDirectory/xxkxxk/xxxxB/xx.xxcm/
def generateJsons():
    print('Generating json files')
    filesTotal = 0
    dirswithJson = 0
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        #print(currentPath) -- DEBUG
        #print(subdirs) -- DEBUG
        #print('\n') -- DEBUG
        transmissionsLineJsonArray = []
        for filename in files:
            if not (filename == 'result.json' or filename == '100BytesString.txt' or filename == '500BytesString.txt' or
                    filename == '1000BytesString.txt'):
                filesTotal += 1
                with open(f'{currentPath}/{filename}') as file:
                    transmissionsLines = file.readlines()
                    frequency = f'{filename}'   # arquivo atual
                    distance = f'{os.path.basename(currentPath)}'   # dir acima Dist
                    bytes = f'{os.path.basename(os.path.dirname(currentPath))}'     # dir acima acima Bytes
                    voltageDivider = f'{os.path.basename(os.path.dirname(os.path.dirname(currentPath)))}'   # dir acima acima VoltageDivider
                    cleanTransmissions = 0      # 0 erros
                    dirtyTransmissions = 0      # 0 < errorRate =< 0.050
                    failedTransmissions = 0     # 0.050 < errorRate

                    # For each trnamission, we're going to generate one json object
                    # All json wil be put in a json array
                    if(transmissionsLines and len(transmissionsLines) >= 10):
                        transmissionsLineDict = {}
                        transmissionsLineDict.update({'voltageDivider': f'{voltageDivider}'})
                        transmissionsLineDict.update({'bytes': f'{bytes}'})
                        transmissionsLineDict.update({'distance': f'{distance}'})
                        transmissionsLineDict.update({'frequency': f'{frequency[:-4]}'})
                        for index in range(0, 10):
                            if ((f'{os.path.basename(os.path.dirname(currentPath))}') == '0100B'):
                                if(transmissionsLines[index][16:-1] == String100Bytes):
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}Error': '0/100'})
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}ErrorRatio': '0'})
                                    cleanTransmissions += 1
                                else:
                                    error = 0
                                    errorRate = .0
                                    for strIndex in range(0, 100):
                                        if not(transmissionsLines[index][16:-1][strIndex] == String100Bytes[strIndex]):
                                            error += 1
                                    errorRate = round(error/100, 3)
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}ErrorRatio': error})
                                    transmissionsLineDict.update(
                                        {f'transmissionNumber{index+1}ErrorRatio': errorRate}
                                    )
                                    if(errorRate <= 0.050):
                                        dirtyTransmissions += 1
                                    else:
                                        failedTransmissions += 1
                            elif ((f'{os.path.basename(os.path.dirname(currentPath))}') == '0500B'):
                                if(transmissionsLines[index][16:-1] == String500Bytes):
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}Error': '0/500'})
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}ErrorRatio': '0'})
                                    cleanTransmissions += 1
                                else:
                                    error = 0
                                    errorRate = .0
                                    for strIndex in range(0, 500):
                                        if not(transmissionsLines[index][16:-1][strIndex] == String500Bytes[strIndex]):
                                            error += 1
                                    errorRate = round(error/500, 3)
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}ErrorRatio': error})
                                    transmissionsLineDict.update(
                                        {f'transmissionNumber{index+1}ErrorRatio': errorRate}
                                    )
                                    if(errorRate <= 0.050):
                                        dirtyTransmissions += 1
                                    else:
                                        failedTransmissions += 1
                            elif ((f'{os.path.basename(os.path.dirname(currentPath))}') == '1000B'):
                                if(transmissionsLines[index][16:-1] == String1000Bytes):
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}Error': '0/1000'})
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}ErrorRatio': '0'})
                                    cleanTransmissions += 1
                                else:
                                    error = 0
                                    errorRate = .0
                                    for strIndex in range(0, 1000):
                                        if not(transmissionsLines[index][16:-1][strIndex] == String1000Bytes[strIndex]):
                                            error += 1
                                    errorRate = round(error/1000, 3)
                                    transmissionsLineDict.update({f'transmissionNumber{index+1}Error': error})
                                    transmissionsLineDict.update(
                                        {f'transmissionNumber{index+1}ErrorRatio': errorRate}
                                    )
                                    if(errorRate <= 0.050):
                                        dirtyTransmissions += 1
                                    else:
                                        failedTransmissions += 1
                        transmissionsLineDict.update({'cleanTransmissions': f'{cleanTransmissions}'})
                        transmissionsLineDict.update({'dirtyTransmissions': f'{dirtyTransmissions}'})
                        transmissionsLineDict.update({'failedTransmissions': f'{failedTransmissions}'})
                        if(cleanTransmissions+dirtyTransmissions < 10):
                            cleanDirtyTransmissions = f'0{cleanTransmissions+dirtyTransmissions}'
                        else:
                            cleanDirtyTransmissions = cleanTransmissions+dirtyTransmissions
                        transmissionsLineDict.update(
                            {'overallTransmissions': f'{cleanDirtyTransmissions}/10'})
                        #firstFailedFrequencyTrnamission = False
                        if (cleanTransmissions == 10):
                            transmissionsLineDict.update({'resultTransmission': 'SUCCESS'})
                        elif (cleanTransmissions+dirtyTransmissions >= 9):
                            transmissionsLineDict.update({'resultTransmission': 'ACCEPTABLE'})
                        else:
                            transmissionsLineDict.update({'resultTransmission': 'FAILED'})
                            #firstFailedFrequencyTransmission = True

                        transmissionsLineJson = json.dumps(transmissionsLineDict, indent=4)
                        transmissionsLineJsonArray.append(transmissionsLineJson)
                        #print(transmissionsLineJsonArray) -- DEBUG

                        # If 2 or more trasmission from 10 failed, ignore the following frequencies for that distance
                        # if(firstFailedFrequencyTransmission):
                        #    break;
                    else:
                        print("Less than 10 transmissions")
                        print(f'{currentPath}/{filename}')
                    # exit(0) -- DEBUG
                    file.close()
        # Since transmissionsLineJsonArray will have data only inside folders with files. Since we have files only in
        # the lowest folders in the tree (leaves), we can ensure that condition
        if(transmissionsLineJsonArray):
            # Save json data along Hz files
            file = open(f'{currentPath}/result.json', "w")
            file.write('[\n')
            for jsonData in transmissionsLineJsonArray[:-1]:
                file.write(jsonData)
                file.write(",\n")
            file.write(transmissionsLineJsonArray[-1])
            file.write('\n]')
            file.close()
            dirswithJson += 1
            #print(transmissionsLineJsonArray) -- DEBUG
            #print(currentPath) -- DEBUG
        # If transmissionsLineJsonArray is empty, we're in a directory with no raw data. If there is any result.json,
        # it SHOULD NOT be there
        else:
            try:
                os.remove(f'{currentPath}/result.json')
            except OSError:
                pass

    print('\nAll json files created')
    # Files quantity verified on Windows Files Explorer
    print(f'Scanned Files {filesTotal}')


# Get all rootDirectory/xxkxxk/xxxxB/xx.xxcm/ and compiles in the file rootDirectory/result.json
def compileJsons():
    print('\nCompiling jsons to a json file in root directory')
    transmissionsLineJsonArray = []
    dirswithJson = 0
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        #print(currentPath) -- DEBUG
        #print(subdirs) -- DEBUG
        #print('\n') -- DEBUG

        for filename in files:
            if (filename == 'result.json' and not currentPath == rootDirectory):
                #print(currentPath) -- DEBUG
                with open(f'{currentPath}/result.json') as resultJson:
                    dict = json.loads(resultJson.read())
                    for data in dict:
                        transmissionsLineJson = json.dumps(data, indent=4)
                        transmissionsLineJsonArray.append(transmissionsLineJson)
                        #print(data) -- DEBUG
                resultJson.close()
                #dirswithJson+=1 --DEBUG

    #print(transmissionsLineJsonArray) -- DEBUG
    file = open('C:/Users/Daniel/Documents/VLC_DADOS_CLEAN/result.json', "w")
    file.write('[\n')
    for jsonData in transmissionsLineJsonArray[:-1]:
        file.write(jsonData)
        file.write(",\n")
    file.write(transmissionsLineJsonArray[-1])
    file.write('\n]')
    file.close()
    #print(dirswithJson) --DEBUG
    print('\nAll json files compiled to root directory')


# Open root Json for analysis
def openJson():
    with open(f'{rootDirectory}/result.json') as resultJson:
        #data = json.loads(line) for line in resultJson
        dict = json.loads(resultJson.read())
        print(len(dict))
        voltageDivider = '50k01k'
        bytes = '1000B'
        #distance = '04.5cm'
        #print(f'{voltageDivider} > {bytes} > {distance}')
        actualDistance = ''
        previousDistance = ''
        amount = 0
        for data in dict:
            actualDistance = data["distance"]
            if(data["voltageDivider"] == voltageDivider
               and data["bytes"] == bytes
               and data["resultTransmission"] == "FAILED"
               and data["frequency"] == "1000Hz"):
                if(actualDistance != previousDistance):
                    print('')
                    print(data['distance'])
                outputResultSearch = ''
                outputResultSearch += f'{data["frequency"]}'
                if (len(data["cleanTransmissions"]) == 1):
                    outputResultSearch += f' 0{data["cleanTransmissions"]}'
                else:
                    outputResultSearch += f' {data["cleanTransmissions"]}'
                if (len(data["dirtyTransmissions"]) == 1):
                    outputResultSearch += f' 0{data["dirtyTransmissions"]}'
                else:
                    outputResultSearch += f' {data["dirtyTransmissions"]}'
                if (len(data["failedTransmissions"]) == 1):
                    outputResultSearch += f' 0{data["failedTransmissions"]}'
                else:
                    outputResultSearch += f' {data["failedTransmissions"]}'
                outputResultSearch += f' {data["overallTransmissions"]}'
                outputResultSearch += f' {data["resultTransmission"]}'
                print(outputResultSearch)
                amount = amount + 1
                print(amount)
                previousDistance = data["distance"]
    resultJson.close()


def deleteJsons():
    for currentPath, subdirs, files in sorted(os.walk(rootDirectory)):
        for filename in files:
            if (filename == 'result.json'):
                try:
                    os.remove(f'{currentPath}/result.json')
                except OSError:
                    pass
    #print(transmissionsLineJsonArray) -- DEBUG
    print('\nAll json files deleted')


if (__name__ == '__main__'):
    # !!! RENAME AND ADJUST FILENAMES TO GET SORT CORRECTED - CAREFUL !!!
    # DO NOT NEED TO BE EXECUTED. MADE JUST FOR EASY READING WHEN FILTER JSONS
    #renameToMilimeters()
    #renameToCentimeters()
    #adjustBytesName()
    #adjustCentimetersName()
    #adjustFrequencyName()
    # VoltageDivider directory its easier to adjust manually


    #generateJsons()
    #compileJsons()
    openJson()
    #deleteJsons()

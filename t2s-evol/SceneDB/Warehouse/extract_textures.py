# -*- coding: utf-8 -*-
"""
Created on Wed Nov 15 21:26:55 2017

@author: ruim
"""

import re
import glob
import shutil
import os


def collect_filenames(filetype):
    filenames = list()
    for name in glob.glob('*.' + filetype):
        filenames.append(name)
    return filenames

def process_texture_file(mtlBaseName):
    mtlFilename = mtlBaseName + '.mtl'
    mtlFile = open('./' + mtlFilename , 'r')
    fileLines = mtlFile.readlines()
    pos = mtlFilename.find('.')
    mtlBaseName = mtlFilename[:pos]
    
    newMtlFile = open('./scenes/' + mtlFilename, 'w')
    newLine = ''
    newTextureFullPath = ''
    
    for currLine in fileLines:
        if currLine.find('map_Ka') != -1:
            textureFullPath = currLine[8:-1]
            pos = currLine.rfind('\\')
            if pos != -1:
                textureName = currLine[pos+1:-1]
                texturePath = currLine[8:pos+1]
            else:
                textureName = currLine[8:-1]
                texturePath = './'
           
            newTextureName = mtlBaseName + '_' + textureName
            #print(textureFullPath)
            newTextureFullPath = './textures/' + newTextureName
            if os.path.exists(textureFullPath) and not os.path.exists(newTextureFullPath):                              
                shutil.copyfile(textureFullPath, newTextureFullPath)
            
            newLine = '\tmap_Ka .' + newTextureFullPath + '\n'
        elif currLine.find('map_Kd') != -1:
            newLine = '\tmap_Kd .' + newTextureFullPath + '\n'
        else:
            newLine = currLine
        
        newMtlFile.write(newLine)
    
def process_scenes(objFileNames):
    for name in objFileNames:
        pos = name.find('.')
        basename = name[:pos]
        objFullName = './' + basename + '.obj'
        newObjFullName = './scenes/' + basename + '.obj'
        if os.path.exists(objFullName) and not os.path.exists(newObjFullName):
            shutil.move(objFullName, newObjFullName)
            process_texture_file(basename)
        

def main():
    objFileNames = collect_filenames('obj')
    
    for name in objFileNames:
        print(name)
    
    
    process_scenes(objFileNames)
    
if __name__ == "__main__": main()
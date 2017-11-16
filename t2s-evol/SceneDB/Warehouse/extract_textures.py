# -*- coding: utf-8 -*-
"""
Created on Wed Nov 15 21:26:55 2017

@author: ruim
"""

import re
import glob


def collect_mtl_filenames():
    mtlFilenames = list()
    for name in glob.glob("*.mtl"):
        mtlFilenames.append(name)
    return mtlFilenames

def copy_texture_files(mtlFilenames):
    for name in mtlFilenames:
        mtlFile = open('./' + name, 'r')
        fileLines = mtlFile.readlines()
        pos = name.find('.')
        mtlBaseName = name[:pos]
        
        for currLine in fileLines:
            if currLine.find('map_Ka') != -1:
                texture_full_path = currLine[8:]
                pos = currLine.rfind('\\')
                if pos != -1:
                    texture_name = currLine[pos+1:]
                    texture_path = currLine[8:pos+1]
                else:
                    texture_name = currLine[8:]
                    texture_path = './'
               
                texture_name = mtlBaseName + '_' + texture_name
                print(texture_name)
                


def main():
    mtlFilenames = collect_mtl_filenames()
    
    """   for name in mtlFilenames:
        print(name)
    """

    copy_texture_files(mtlFilenames)
    
if __name__ == "__main__": main()
############################################################################################
# This is the Demo code for Scene Generation Module of
# "Language-Driven Synthesis of 3D Scenes from Scene Databases"
# Rui Ma, Akshay Gadi Patil, Matthew Fisher, Manyi Li, Sören Pirk,
# Binh-Son Hua, Sai-Kit Yeung, Xin Tong, Leonidas Guibas, Hao Zhang
# ACM Transactions on Graphics (Proc. SIGGRAPH ASIA), 2018
# The code is free for research use. For commerical use, please contact: Rui Ma(maruitx@gmail.com)
# Version: 1.0
############################################################################################

Step 0: Required Software
QT 5
Visual Studio 2015
Qt Visual Studio Tools
Compiled NLP module from downloaded NLP_release code

Step 1: Setup environment variable for third party software
1) add path of freeglut.dll (SceneGen_release/ThirdParty/freeglut-MSVC-3.0.0-2.mp/freeglut/bin/x64) to the system's path
2) add path of glew32.dll (SceneGen_release/ThirdParty/glew-1.12.0/bin/Release/x64) to the system's path

Step 2: Configure NLP module
1) copy the following files into SceneGen_release folder
	a. complied NLP parsing program, e.g., NLP_release/matt-SEL-playground/x64/Release/matt-SEL-playground.exe
	b. NLP_release/matt-SEL-playground/SEL-params.txt (Make sure nlpRoot has pointed to the NLP_release path in your computer)
2) Start NLP server in the background using IntelliJ. (Refer to the README.txt in NLP_release folder)
3) To check whether the NLP program is successfully configured, you can create a in.txt file with a test sentence in SceneGen_release folder and run the copied matt-SEL-playground.exe to see if an out.txt file is created in SceneGen_release folder

Step 3: Configure scene data base path to downloaded SceneDB folder
1) In SceneGen_release/SceneDB/LocalSceneDBPath.txt, set path of StanfordDB to the downloaded folder SceneDB/StanfordSceneDB
2) The path of ResultPath specifies where the generated scenes will be saved when triger "ss SceneName" command in the text dialog

Step 4: Compile and Run
1) In Visual Studio 2015, on the tool bar, click QS VS TOOLS->Open Qt Project File (.pro), then choose SceneGen_release/main.pro. Then the solution main.sln will be automatically generated. 
2) Build the solution in release mode. Then press Ctrl+F5 to start the program. Also, make sure the NLP server is running in the background (Step 2).
3) A text dialog, a GUI window and a console window will appear when program starts (it may take about 10 seconds for loading the relational models). 
4) In the text dialog, enter the sentence and press "Process". Generate 3D scenes will be shown in the GUI window. The GUI window contains a main view on the left and five previews on the right. Click the preview on the right panel can switch the scene shown in the main view. You can also using the mouse to scroll down to see more scene previews.

Suggested workflow of using the scene generation tool:
1) In the text dialog, enter a sentence and press "Process" to generate scenes for the given sentence. Note,
	a. try to describe objects and their relationships in the sentence; see sample sentences below
	b. use one sentence in each step; in the later steps, try to use some existing object as reference and describle new objects based on their relationships to the reference object
2) Browse the results and select a scene of interest. Then, enter another sentence in the dialog and press "Process" to continue evolving from current selected scene.
3) Repeat 1) and 2) for several steps; in the text dialog, enter ss SceneName to save current selected scene


When the generated results are not ideal:
1) In the text dialog, enter clear to clear all generated scenes
2) In the text dialog, enter ls SceneName.result to load the saved scene 
3) After loading a scene using ls SceneName.result, enter new sentence in the text dialog to continue evolve from currently loaded scene

Parameters
UseContext: whether to add the additional objects (objects which are not mentioned in the input sentence) based on the contextual information extracted from the scene database; value is 0 or 1; default is 0
ContextCoOccProb: co-occurrence threshold of adding contextual objects; value from 0.1 to 1 (set the value too small may introduce many less relevant objects and may increase layout generation time due to more collision detection); default is 0.5; enabled when UseContext=1
GroupOccProb: occurrence threshold of adding active objects in the group relational model; value from 0.1 to 1 (set the value too small may introduce many less relevant objects and may increase layout generation time due to more collision detection); default is 0.5

Sample sentences:
1) Desk
There is a desk with two monitors, a keyboard, and a mouse.
A cellphone, a headphone and a lamp are on the desk.
Replace the desk. (note: for verb commands, only the current selected view will execute the command)

2) Office
There is an organized computer desk.
Next to the desk, there is a file cabinet with a printer on top.
A bookshelf with books is to the right of the desk. 

3)Bedroom
There is a bed.
Next to the bed, there is a messy office desk.
There are books scattered on the bed.

4)Dining room
There are four chairs around a round dining table.
Put a vase and a teapot on the table.
The table is messy.
Move the chairs apart. (note: for verb commands, only the current selected view will execute the command)

5) Living room
There is a couch and two sofa chairs in the room.
In front of the couch is a messy coffee table. 
In front of the couch, there is a tv with two speakers on each side.

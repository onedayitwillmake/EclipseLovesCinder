#Xcode is a great IDE, however it is not great at C++ editing

It provides only very barebones syntax highlighting, even more barebones code completion, and no edit-time error checking.


Cinder is a great library, but it's very vast - Without **code-completion** I was very lost/ nearly all the time poking through the header files and opening the .cpp files in other editors so I could learn it more thouroughly.


## (Re)-Enter EclipseIDE for C++
This project is presented as is for anyone else who would like to use a C++ editor on osx that is actually aware of your classes.  
From what I've used it so far, it is much better - but compile times are a little slower, however the time you save while editing is well worth it imo.

## Usage 
1. http://www.eclipse.org/downloads/packages/eclipse-ide-cc-developers/heliossr2
2. Download this project
3. Import into eclipse workspace (select import existing project in the 'general' drop down)  
4. RightClick project root, select properties
   1. Select C/C++ Build
   2. Select Settings
   3. Go through the drop downlist, and replace '/Users/onedayitwillmake/GIT/LIBRARY/Cinder/' with your own cinder path
5. Select **C/C++ General**
   1. Select Paths and Symbols
   2. Select GNU C++
   3. Replace references to '/Users/onedayitwillmake/GIT/LIBRARY/Cinder/' with your own cinder/boost path
6. After hitting, apply/ok and closing that window select Project->Build All and project should build successfully.

qtchan is a 4chan browser written with qt5.\
I started developing it because browsers suck.\
There's still a lot to do.


# Features
* browse and post on 4chan
* auto update threads and auto download original file
* download all media as original filename
* tree style tabs and save state (./session.txt)
* open files in OS's default application
* search and filter
* filter by regexp$options
* self-archive
* (You)

### Build Instructions (Linux/BSD based)
needs qt5
```
git clone https://github.com/siavash119/qtchan 
cd qtchan
mkdir build
cd build
qmake ../
make
./qtchan
```

Archlinux:
```
makepkg --install
```
or from AUR:
```
packer -S qtchan
```


## a few keyboard commands and instructions
* ctrl+l opens up the nav bar; you can type g or g/boardnum or /g/ or https://boards.4chan... etc.\
g for first page, /g/ for catalog, g/search to get catalog and search
* o opens thread/image at top of view (expand the tree if you don't see the new tab in the treeview)
* F3 focuses the tree
* F4 focuses the content
* #j and #k scroll (e.g. 5j, k)
* #h and #l scroll by (You) replies
* #G puts scrollbar at percent (e.g. G for bottom, 0G for top, 50G for 50%)
* ctrl+w (ctrl+f4 on windows) or delete to close tab
* ctrl+p opens settings-ctrl+tab ctrl+shift+tab, ctrl+1-4 to switch tabs
* ctrl+f to search and filter
* r manually refreshes thread
* q opens up post form, shift+enter to post from there
* focus the bottom lineEdit to load captcha; press enter to verify/reload
* e expands all images and auto expands future posts on current tab
* ctrl+plus/minus to zoom in/zoom out text
* ctrl+9/0 to scale down/up images
* ctrl+q to quit
* F7 reloads filters.conf file
* F8 opens self-archive window; F5 reloads it
* F9 toggles notification view
* F10 saves tree state (it'll also save on exit)
* F11 hides menubar (might disable some shortcuts)
* if you have mpv, g opens all the images/videos in the thread.

there are more shortcuts; look at the source for now for info.

need to restart qtchan if changing to use 4chan pass or not. signals/slots still not finished for that.

you can make a filter file and it'll regexp remove posts for you. I'll post an example

## Captcha
1. in the post form, focus the bottom lineEdit. It will load the challenge image
2. it's a grid following the numpad:
* 7 8 9
* 4 5 6
* 1 2 3
3. type in the numbers and hit enter (e.g. 034)
4. if it says "Verified.", you're good to post.
* Your code will expire in 2 minutes. If you want to load a new captcha, hit enter in the lineEdit or click the image.

Note: the numbers in this image follow a previous captcha number grid, the correct answer would be 893 in any order.
![qtchan captcha](https://i.abcdn.co/qtchan_captcha.png)

## Filters
Create a file ~/.config/qtchan/filters.conf
```
!key
regex (don't surround with or escape forward slashes; always case insensitive)
regex
regex$option:value,value;option:value
#comment
!

!sub
/yourbadgeneral/$boards:g,pol;op:only
##all generals
general$op:only
!

!name
##name users
^(?!Anonymous$)$
!

!filename
##fap
#^$$
##work
#^(?!$)$
!
```
F7 reloads and applies the filters.conf file

## Using 4chan pass to post
Create a file ~/.config/qtchan/cookies like below. Look at your browser cookies for values.
Open settings with Ctrl+P and check Use 4chan pass.
```
4chan_pass:your_4chanpass
pass_id:your_pass_id
pass_enabled:1
```

## Extra flags for int
In ~/.config/qtchan/qtchan.ini add these lines. Don't put your country name:
```
[extraFlags]
enable=true
region=YourRegion||SubRegion||another
```

![qtchan ss](https://i.abcdn.co/qtchan.png)


## TODO
### MAJOR
* better filter support
* unread, read display
* keybinds for prev/next yourPost
* better notifications
* better settings
* better instructions

### MINOR
* support other chans
* fix catalogue view (i.e. /boardname/; boardtab clears after almost finishing loading)
* show loading state
* themes
* more settings

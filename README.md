qtchan is a 4chan browser written with qt5.\
I started developing it because browsers suck.\
There's still a lot to do.

# Features
* browse and post (only 4chan pass right now) on 4chan
* tree style tabs and save state
* download all media as original filename
* open files in OS's default application
* search and filter

## Using 4chan pass to post
Create a file ~/.config/qtchan/settings like below. Look at your browser cookies for values.
```
{
	"cookies":{
		"4chan_pass":"your 4chanpass",
		"pass_id":"your pass id",
		"pass_enabled":"1"
	}
}
```

![qtchan ss](https://i.abcdn.co/qtchan.png)

## TODO
### MAJOR
* captcha
* better geometry for hovered and embedded replies
* instructions
* settings

### MINOR
* save tree tabs order and structure
* show loading state
* load pixmaps in another thread?
* themes

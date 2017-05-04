qtchan is a 4chan browser written with qt5.\
I started developing it because browsers suck.\
There's still a lot to do.
## TODO
### MAJOR
* load pixmaps in another thread
* instructions
* captcha
* settings

### MINOR
* themes

![qtchan ss](https://i.abcdn.co/qtchan.png)

# Using 4chan pass to post
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

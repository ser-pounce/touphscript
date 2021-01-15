# ToughScript

## Additional configuration for 1.4

Latest 1.4 version is missing some text files. These are inspired from Reunion R06 installer and should work with this 1.4:

Download the following files a place them in the same folder as the `toughScript.exe`:
- [Chars.txt](Chars.txt)
- [touphScript.ini](touphScript.ini)

In `touphScript.ini` configure the paths to your game files eg: 

```properties
#Override paths
text = .\text\
flevel = NONE
world = NONE
scene = "D:\Steam\steamapps\common\Final Fantasy VII\data\lang-en\battle\scene.bin"
kernel = "D:\Steam\steamapps\common\Final Fantasy VII\data\lang-en\kernel\kernel.bin"
kernel2 = "D:\Steam\steamapps\common\Final Fantasy VII\data\lang-en\kernel\kernel2.bin"
window = "D:\Steam\steamapps\common\Final Fantasy VII\data\lang-en\kernel\window.bin"
exe = NONE
```

Now the `toughScript.exe` will run
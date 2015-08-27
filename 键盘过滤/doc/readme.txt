键盘IRP必然要经过kbdclass，不管是刚插上来的。

因此过滤kbdclass就可以了。

我们的设备是NT设备，估计也就不会调用我们的AddDevice，和PNP
因此，这两个部分可以在实验中并没有体现出效果

kbdclass的设备和实际的键盘设备在同一个栈上吗？
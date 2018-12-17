# MagicMirror

This is a project conducted by our group, in Intelligence Image Process Lab, SJTU.
<br> It performed arbitrary style transfer in video flow, based on Kinect II. 
<br> The project supported `sytle transfer`,`gesture recognition`, `foreground extraction` and `age recognition`.
[DEMO](https://www.bilibili.com/video/av36757803 "悬停显示") 


Main Modules
-------------------

MagicMirror/

* **BodyDetect**
<br>Detect the numbers of users and their index.

* **BodyInfo**
<br> Provide information of users: coordinates of joints, index of users, gesture status.

* **Style Transfer**
<br> Perform style transfer based on AdaIN model.



* **MsgQue**
<br> Multithread

* **classifier**
<br> Predict age of users presented.

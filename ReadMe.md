##Proposal for Course Project: TCP Network Storage       
###(Proposal has been changed with the final program)

Team members: Hao Zhou , Pei Zhang , Ya Liu        
We try to implement TCP Network Storage (TNS), which is like DROPBOX to some extent.      

When a user run Client in his computer for the first time, the Client will ask the user using his authorized user name and password to log in to the Server. After that, the Client need the user to set the path of a directory, which the Client will monitor all the time. [*Note: The Client will store the user name and path into a local file, so that the client will no longer ask the user to type them every time it runs.]       

###When the Client is monitoring that directory: [*Real-time monitoring may be challenging.]
Initialization: The Client will communicate with the Server to find the difference between two sides�� files.       
If the Client has new or modified files which have not synchronized by the Server, the Server will receive them from the Client and add or rewrite the files in the server side.      

If the Server has the files which does not exist in the client side, the Client will receive the files from the Server.      
 
###Operations:
If any new file is added into this directory, the Client will tell the Server that update as quick as possible. Then the Server will receive the new added file from the Client.       
      
If a file is modified in the Client, the Server will be told about this by the Client, and rewrite the old file in the Server with the new one received from the Client.       
      
If a file is deleted in the Client, the Client will not send any notice to the Server about the deleting. So the Server has no response to this case.      
       
From above, we know that if we delete the file in the client side, the file will remain in the server side. And when the Client restarts, it will get the file from the Server. In this case, files in the Client can never be deleted. To fix this, we design ��delete FileName�� command to compel the Server to delete file in the server side.        

###Other Commands we will implement:
Login name passwd;	ModifyPasswd [Old Passwd];	ModifyPath [New Path];    
Register name passwd;	stop synchronize; 	start synchronize; 	quit...    

###Further work:(not implemented)
User can share their files with others [Public or Particular users].       
Commands designed for this:       
Share FileName [OtherUserName];  Download FileName;	list... [*Note: List command will return all the files available to the logged user, which will include other users�� files shared.]

--------------------
>1. "CourseProject" is for final project.
>2. "Go_Back_N_simple_demo" is a simple demo for Go Back N, as we couldn't add it to our final program.

There are still many bugs! I will try to fix it when I find one.


You can find more information about this code with the url below:
http://web.cs.wpi.edu/~rek/Grad_Nets/Spring2013/CourseProject_S13.pdf

Thanks
Team 10

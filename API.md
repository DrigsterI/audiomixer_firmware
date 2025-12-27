Device communication protocol
=============================

1 byte for command  
1 byte for target(usualy channel)  
any number of bytes for value  

Commands
--------

#### To device
0x01 - Request info  
0x02 - Set volume  

#### From device
0x81 - Send info  
0x82 - Send volume  
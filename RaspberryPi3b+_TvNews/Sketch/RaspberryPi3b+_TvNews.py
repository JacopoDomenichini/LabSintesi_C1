############################
#          Sockets
############################
import socket
#receive socket settings
local_UDP_IP = "192.168.4.1"
local_UDP_PORT = 5005
rxData = ""
empty = ""
global rxSock
rxSock = socket.socket(socket.AF_INET, # Internet
                 socket.SOCK_DGRAM) # UDP
rxSock.bind((local_UDP_IP, local_UDP_PORT))
rxSock.setblocking(0) #sets the udp as non blocking
# receive function definition
def receive(): #function to read data if are present in buffer
    try:
        data, addr = rxSock.recvfrom(1024) # get data from buffer,
    except socket.error:# if empty asserts an error clear the data and pass to return
        data = ""
        pass   
    return data

#transmit socket settings
remote_UDP_IP = "192.168.4.2"
remote_UDP_PORT = 2390
global txSock
txSock = socket.socket(socket.AF_INET, # Internet
                 socket.SOCK_DGRAM) # UDP
txSock.setblocking(0) #sets the udp as non blocking
#Transmit function definition
def transmit(data,UDP_IP,UDP_PORT):
    data_as_bytes = str.encode(data)
    txSock.sendto(data_as_bytes,(UDP_IP,UDP_PORT))

############################
#    VLC Video Player
############################
import vlc
import time
#import keyboard

# Videos path location
videoLoop = "/home/tempeh/Desktop/ICE/Video/glitch loop per raspberry resized.mp4" #index (0) in media player list
video1 ="/home/tempeh/Desktop/ICE/Video/VIDEO 1.mp4"  #index (1) in media player list
video2 ="/home/tempeh/Desktop/ICE/Video/VIDEO 2.mp4"  #index (2) in media player list
video3 ="/home/tempeh/Desktop/ICE/Video/VIDEO 3.mp4"  #index (3) in media player list
video4 ="/home/tempeh/Desktop/ICE/Video/VIDEO 4.mp4"  #index (4) in media player list
video5 ="/home/tempeh/Desktop/ICE/Video/VIDEO 5.mp4"  #index (5) in media player list
video6 ="/home/tempeh/Desktop/ICE/Video/VIDEO 6.mp4"  #index (6) in media player list
video7 ="/home/tempeh/Desktop/ICE/Video/VIDEO 7.mp4"  #index (7) in media player list
video8 ="/home/tempeh/Desktop/ICE/Video/VIDEO 8.mp4"  #index (8) in media player list
video9 ="/home/tempeh/Desktop/ICE/Video/VIDEO 9.mp4"  #index (9) in media player list
video10 ="/home/tempeh/Desktop/ICE/Video/VIDEO 10.mp4"  #index (10) in media player list

# VLC Library instances creation and settings
# creating Instance class object
vlc_instance = vlc.Instance()

# creating a media list player object
media_list_player = vlc.MediaListPlayer()

# creating vlc media player object
media_player = vlc.MediaPlayer()

# setting full screen status for the media player object
media_player.set_fullscreen(True)

# set the media player object in to the media list player object included all its options
media_list_player.set_media_player(media_player)

# creating a new media list object
media_list = vlc_instance.media_list_new()

###################################
#Load videos in media list sequence
###################################
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(videoLoop)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
# set for video 0 loop option
#vlc_instance.vlm_set_loop(videoLoop, True)

# creating a new media steps for each new video in media player List
media = vlc_instance.media_new(video1)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)

# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video2)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)

# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video3)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)

# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video4)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
        
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video5)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
        
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video6)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
        
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video7)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
        
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video8)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
        
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video9)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)
        
# creating a new media steps for each new video in mediaList
media = vlc_instance.media_new(video10)
# adding media to media list
media_list.add_media(media)
# setting media list to the media player
media_list_player.set_media_list(media_list)

#State machine initial state setting
state = 10
videoIndex = 1

while True:
       
    if state == 10:
        # start playing video 0 for loop
        media_list_player.play_item_at_index(0)
        loopStop_sec = time.monotonic() + 10
        print("Stato: %s" % loopStop_sec)
        state = 20

    if state == 20:
        # listen for UDP message from arduino
        rxData = receive()
        # when UDP message arrives goes at State 30 to play  another Video
        if rxData != empty:
            print("received message: %s" % rxData)
            state = 30
        # if no UDP message arrives before 11seconds retarts the viedo 0 for the loop at state 10
        actual_sec = time.monotonic()
        if actual_sec > loopStop_sec:
            state = 10
            
    if state == 30:
        # start playing video
        media_list_player.play_item_at_index(videoIndex)
        # increase the to play Video Index for next time
        videoIndex = videoIndex + 1
        if videoIndex > 10:
            videoIndex = 1
        print("Stato: %s" % state)
        state = 40

    if state == 40:
        # after the videao has started gets the playing video length in ms 
        time.sleep(0.5)
        videoLength = (media_player.get_length() / 1000) - 1
        print("Lunghezza: %s" % videoLength)
        
        time.sleep(videoLength)
        txData = "finished"
        transmit(txData,remote_UDP_IP,remote_UDP_PORT)
        print("Stato: %s" % state)
        state = 10
        
    


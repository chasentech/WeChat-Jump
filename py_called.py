import os

def add_func(x, y): 
	re = x + y
	print(re)

def press_value(x1, y1, x2, y2, dist):
	    os.system('adb shell input swipe ' + str(x1) + ' '+ str(y1) + ' ' + str(x2) + ' '+str(y2) + ' ' + str(dist))
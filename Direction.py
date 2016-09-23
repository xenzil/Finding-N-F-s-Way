import math
import networkx as nx

def checkCurrDirection(north, compass):
    currDirection = (int(compass) + north) % 360
    return currDirection

def checkDestDirection(cn, nn):
    cx = int(cn['x'])
    cy = int(cn['y'])
    nx = int(nn['x'])
    ny = int(nn['y'])

    dx = nx - cx
    dy = ny - cy
    
    if dx > 0:
        destDirection = 90

    elif dx < 0:
        destDirection = 270

    elif dy > 0:
        destDirection = 0

    elif dy < 0:
        destDirection = 180

    return destDirection

def calcRotation(curr, dest):
    diff = dest - curr
    rotate = None
    if diff > 0:
        if diff > 180:
            rotate = 'anti-clockwise ' + str(360 - diff)

        else:
            rotate = 'clockwise ' + str(diff)

    else:
        if diff > -180:
            rotate = 'anti-clockwise ' + str(abs(diff))

        else:
            rotate = 'clockwise ' + str(360 + diff)

    return rotate
    

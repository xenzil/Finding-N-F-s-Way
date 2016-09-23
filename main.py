import urllib2
import json
import math
import networkx as nx
import Direction

###### FUNCTIONS



#### FUNCTIONS FOR MAP


def checkSameMap(currMapURL, endMapURL):
    if (currMapURL == endMapURL):
        return True
    else:
        return False

def getMapUrl(bNum, lNum):
    mapURL = 'http://showmyway.comp.nus.edu.sg/getMapInfo.php?Building=' + bNum + '&Level=' + lNum
    return mapURL

def addNodes(currMap, currNxMap):
    for node in currMap:
        id0 = node.get('nodeId')
        Name = node.get('nodeName')
        xVal = node.get('x')
        yVal = node.get('y')
        currNxMap.add_node(int(id0), name=str(Name), x=str(xVal), y=str(yVal))
    
    return node, id0, Name, xVal, yVal, name, x, y

def addEdges(currMap, currNxMap, node, id0, int):
    for node in currMap:
        for edge in node.get('linkTo').split(","):
            edge = edge.strip()
            id0 = node.get('nodeId')
            dist = int(distBtwNodes(currNxMap.node[int(id0)], currNxMap.node[int(edge)]))
            currNxMap.add_edge(int(id0), int(edge), weight=int(dist))
    
    return weight, node

def getStartEndCoords():
    bNumStart = 'COM1' 
#raw_input('Starting building: ')
    lNumStart = '2'
#raw_input('Starting level: ')
    sourceT = '1'
#raw_input('Starting node: ')
    bNumEnd = 'COM1'
#raw_input('Ending building: ')
    lNumEnd = '2'
#raw_input('Ending level: ')
    targetT = '4'
    return bNumStart, lNumStart, bNumEnd, lNumEnd, sourceT, targetT

def distBtwNodes(n1, n2):
    x1 = int(n1['x'])
    x2 = int(n2['x'])
    y1 = int(n1['y'])
    y2 = int(n2['y'])

    xD = int(x1-x2)
    xDF = int(math.fabs(xD))
    xDS = int(math.pow(xDF, 2))

    yD = int(y1-y2)
    yDF = int(math.fabs(yD))
    yDS = int(math.pow(yDF, 2))

    xyA = int(xDS) + int(yDS)

    sqrt = math.sqrt(xyA)
    
    return sqrt;

###### MAIN


"""

### Map
bNumStart: Starting building
lNumStart: Starting level
nNumStart: Starting node

bNumEnd: Ending building
lNumEnd: Ending level
nNumEnd: Ending node

currMapURL: URL of starting pos map
endMapURL: URL of ending pos map

checkSameMap: bool if same map
isStairs: bool if stairs

currMap: Map of current location
endMap: Map of ending point

"""

"""
### Navigation

currGraphEndNode: End node of current graph

currNode: Current node
nextNode: Next node
finalNode:

"""

"""
### Obstacles

### Sensor readings
usFront / obstacleFront
usRight / obstacleRight
usLeft / obstacleLeft
usDown / obstacleDown

ifrFront

stepsTaken

compass


### others
stepLength (est
stepsNeeded
"""


##### MAIN
shawnAvgStep = 20

bNumStart, lNumStart, bNumEnd, lNumEnd, sourceT, targetT = getStartEndCoords()

currMapURL = getMapUrl(bNumStart, lNumStart)
print currMapURL

endMapURL = getMapUrl(bNumEnd, lNumEnd)
print endMapURL

isSameMap = checkSameMap(currMapURL, endMapURL)

response = urllib2.urlopen(currMapURL)
currMapFull = json.load(response)

#print json.dumps(currMapFull['map'], indent=4, sort_keys=True)

currMap = currMapFull['map']
wifi_map = currMapFull['wifi']
info_map = currMapFull['info']

north = info_map.get('northAt')
    
print str(north)

currNxMap = nx.Graph()
wifiMap = nx.Graph()

for node in currMap:
        id0 = node.get('nodeId')
        Name = node.get('nodeName')
        xVal = node.get('x')
        yVal = node.get('y')
        currNxMap.add_node(int(id0), name=str(Name), x=str(xVal), y=str(yVal))
#print(list(currNxMap.nodes()))

for node in currMap:
        for edge in node.get('linkTo').split(","):
            edge = edge.strip()
            id0 = node.get('nodeId')
            dist = int(distBtwNodes(currNxMap.node[int(id0)], currNxMap.node[int(edge)]))
            currNxMap.add_edge(int(id0), int(edge), weight=int(dist))

#print(list(currNxMap.edges()))

# Getting shortest path
sR = (nx.shortest_path(currNxMap, source=int(sourceT), target=int(targetT), weight='weight'))

print sR

numOfNodesInRoute = len(sR)
print 'num of nodes in route is ' + str(numOfNodesInRoute)

currNode= int(sR[0])
print 'currNode is ' + str(currNode)
nextNode = int(sR[1])
nextNodeIndex = 1

if (isSameMap):
    currGraphEndNode = targetT

print 'currGraphEndNode is ' + str(currGraphEndNode)

while (currNode != currGraphEndNode):
    #calculate steps needed to nextNode
    
    print 'currNode is ' + str(currNode)
    print 'nextNode is ' + str(nextNode)

    distToNextNode = distBtwNodes(currNxMap.node[int(currNode)], currNxMap.node[int(nextNode)])
    print 'distToNextNode is ' + str(distToNextNode)
    stepsNeeded = int(distToNextNode) / shawnAvgStep
    print 'stepsNeeded is ' + str(stepsNeeded)
    obstacleFront = obstacleLeft = obstacleRight = 0
    compass = raw_input('compass: ')
    
    
    #check if facing correct direction
    currDirection = Direction.checkCurrDirection(int(north), int(compass))
    destDirection = Direction.checkDestDirection(currNxMap.node[int(currNode)], currNxMap.node[int(nextNode)])
    degreeToRotate = Direction.calcRotation(currDirection, destDirection)
    
    
    print str(currDirection)
    print str(destDirection)
    print str(degreeToRotate)
    
    while (currNode != nextNode):
        # For testing purposes
        usFront = raw_input('usFront: ')
        usRight = raw_input('usRight: ')
        usLeft = raw_input('usLeft: ')
        izzitNextNode = raw_input('izzit nextNode lolz: ')
        
        if (int(usFront) < 50):
            obstacleFront = 1
        else: 
            obstacleFront = 0
        if (int(usRight) < 100):
            obstacleRight = 1
        else: 
            obstacleRight = 0
        if (int(usLeft) < 100):
            obstacleLeft = 1
        else: 
            obstacleLeft = 0
        
        if (obstacleFront == 1):
            print 'omg got obstacle in front'
        if (obstacleRight == 1):
            print 'right got obstacle'
        if (obstacleLeft == 1):
            print 'left got obstacle'
        if (izzitNextNode == 'y'):
            currNode = nextNode
        """
        if (stepsTaken == stepsNeeded):
            currNode = nextNode
        """
    if (currNode == nextNode):
        if (int(currNode) == int(currGraphEndNode)):
            print 'last node of curr graph reached'
            break
        else:
            nextNodeIndex += 1
            nextNode = sR[int(nextNodeIndex)]
        
        
    

############## WIFI

"""
for node in wifi_map:
    id1 = node.get('nodeId')
    Name = node.get('nodeName')
    xVal = node.get('x')
    yVal = node.get('y')
    macAdd = node.get('macAddr')
    wifiMap.add_node(int(id1), name=str(Name), x=str(xVal), y=str(yVal), macAddr=str(macAdd))
    
    
#print(list(wifiMap.nodes(data=True)))
#print(list(wifiMap.nodes())

"""

#!/usr/bin/env python

###############################################################################
#                                                                             #
# copyright 2013 UNL Holland Computing Center                                 #
#                                                                             #
#  Licensed under the Apache License, Version 2.0 (the "License");            #
#     you may not use this file except in compliance with the License.        #
#    You may obtain a copy of the License at                                  #
#                                                                             #
#        http://www.apache.org/licenses/LICENSE-2.0                           #
#                                                                             #
#    Unless required by applicable law or agreed to in writing, software      #
#  distributed under the License is distributed on an "AS IS" BASIS,          #
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  #
#   See the License for the specific language governing permissions and       #
#   limitations under the License.                                            #
#                                                                             #
###############################################################################

###############################################################################
#                                                                             #
#                           I P G e o P l u g i n                             #
#                                                                             #
###############################################################################

import re
import sys 
import math
import socket
import sqlite3 as lite
import pygeoip

"""
This python module is a modified version of a Larkutilities module, developed
by Andrew Koerner and Bjorn Barrefors.

Modified for xrootd preference plugin by Bjorn Barrefors.

"""

__author__ =  'Andrew B. Koerner'
__email__ =  'AndrewKoerner.b@gmail.com'

__author__ =  'Bjorn Barrefors'

__organization__ = 'Holland Computing Center University of Nebraska - Lincoln'

###############################################################################
#                                                                             #
#                       H e l p e r   F u n c t i o n s                       #
#                                                                             #
###############################################################################
    
def hostIPFix(host_ip):
    # PRE: Domain name in format [::IP]:PORT
    # POST: IP address of domain name
    ip = str(host_ip).strip('[::').partition(']')
    ip = ip[0]
    return ip

def domainToIP(hostname):
    # PRE: Hostname
    # POST: IP address of hostname
    ip = socket.gethostbyname(hostname)
    return ip

def IPToSubnet(ip):
    # Convert IP into Class C subnet by replace everything after last
    # '.' with '0'
    # Pre: A valid IPv4 address
    # Post: Valid Class C IPv4 subnet address
    return re.sub(r'\.\d\d?\d?$', '.0', str(ip).strip())

def coordinateDiff(host_lat, host_long, client_lat, client_long):
    # Find distance between two coordinates
    # Haversine formula
    # Pre: latitude and longitude for two points in str
    # Post: Distance in km between the two points, float format
    R = 6371
    lat1 = float(host_lat)
    lat2 = float(client_lat)
    long1 = float(host_long)
    long2 = float(client_long)
    d_lat = math.radians((lat2-lat1))
    d_long = math.radians((long2-long1))
    lat1 = math.radians(lat1)
    lat2 = math.radians(lat2)
    a = math.sin(d_lat/2)**2 + math.sin(d_long/2)**2 * math.cos(lat1) * math.cos(lat2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    distance = R * c
    return distance

def IPDistance(host_ip, client_domain, database_path):
    # Given two IP addresses, find distance between these in real life
    # Pre: Two IP addresses in str format
    # Post: Distance in km between the two IP addresses in float
    host_subnet = IPToSubnet(hostIPFix(host_ip))
    client_subnet = IPToSubnet(domainToIP(client_domain))
    # Use PygeoIP
    gi4 = pygeoip.GeoIP(str(database_path) + 'GeoLiteCity.dat', pygeoip.MEMORY_CACHE)
    gi6 = pygeoip.GeoIP(str(database_path) + 'GeoLiteCityv6.dat', pygeoip.MEMORY_CACHE)
    host_dict = gi4.record_by_addr(host_subnet)
    if (not host_dict):
        return 1000000
    client_dict = gi4.record_by_addr(client_subnet)
    if (not client_dict):
        return 1000000
    host_lat = host_dict['latitude']
    host_long = host_dict['longitude']
    client_lat = client_dict['latitude']
    client_long = client_dict['longitude']
    distance = coordinateDiff(host_lat, host_long, client_lat, client_long)
    print distance
    return distance

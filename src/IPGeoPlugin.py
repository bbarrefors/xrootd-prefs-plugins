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
import urllib
import urllib2
import BeautifulSoup
import socket
import sqlite3 as lite

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

def normalizeWhitespace(str):
    # Strip leading and trailing whitespace.
    # Make all remaining whitespace (tabs, etc) to spaces.
    # re is imported regular expression object
    # Pre: Any str
    # Post: Stripped leading and trailing whitespaces and all whitespace
    #       like tabs etc are now normal spaces.
    return re.sub(r'\s+', ' ', str.strip())
    
def hostnameToIP(domain):
    # PRE: Domain name in format [::IP]:PORT
    # POST: IP address of domain name
    IP = str(domain).strip('[::').partition(']')
    return IP

def IPToSubnet(IP):
    # Convert IP into Class C subnet by replace everything after last
    # '.' with '0'
    # Pre: A valid IPv4 address
    # Post: Valid Class C IPv4 subnet address
    return re.sub(r'\.\d\d?\d?$', '.0', IP.strip())

def domainToIP(hostname):
    # PRE: Hostname
    # POST: IP address of hostname
    addr = socket.gethostbyname(hostname)
    return addr

def getGeody(IP):
    # Fetch location data from geody
    # Unknown behaviour on IPv6 address, only tested on IPv4
    # Check compatibility on geody.com
    # Pre: An IP address in str format
    # Post: BeautifulSoup data from geody, not str
    geody = "http://www.geody.com/geoip.php?ip=" + IP
    html_page = urllib2.urlopen(geody).read()
    soup = BeautifulSoup.BeautifulSoup(html_page)
    return soup('p')[3]

def cityCountryParser(data):
    # Seperate city and country into two str's
    # Strip all else and return
    # Pre: BeautifulSoup data from geody
    # Post: A str with city, country and optionally state/province.
    #       CITY, [STATE/PROVINCE], COUNTRY
    geo_txt = re.sub(r'<.*?>', '', str(data))
    geo_txt = geo_txt[32:].upper().strip()
    stripped_data = geo_txt.strip("IP: ").partition(': ')
    city_country = stripped_data[2]
    stripped = city_country.partition(' (')
    city_txt = stripped[0]
    return city_txt

def latLong(city_country):
    # Find latitude and longitude of city/country.
    # This function is not extremely accurate yet.
    # Would like to pick best result instead of first.
    # Pre: A str containing city, country and if available state/province. 
    #      CITY, [STATE/PROVINCE], COUNTRY
    # Post: Latitude and longitude of IP address in str format
    geody = "http://www.geody.com/geolook.php?world=terra&map=col&q=" + urllib.quote(city_country) + "&subm1=submit"
    html_page = urllib2.urlopen(geody).read()
    soup = BeautifulSoup.BeautifulSoup(html_page)
    link = soup('a')[10]
    strip1 = str(link).partition('Coords: ')
    strip2 = strip1[2].partition('\"')
    strip3 = strip2[0].partition(',')
    latitude = strip3[0]
    longitude = strip3[2]
    return latitude, longitude

def IPGeolocate(IP):
    # Get data about IP address and parse it to extract city and country.
    # Translate city and country into longitude and latitude coordinates.
    # Pre: IP address in str format
    # Post: Longitude and latitude coordinates for IP address, str format
    raw_data = IPGeoPlugin.getGeody(IP)
    city_country = IPGeoPlugin.cityCountryParser(raw_data)
    print(city_country)
    # Return coordinates for antarctica if unknown position of IP
    if "UNKNOWN" in IPGeoPlugin.normalizeWhitespace(city_country):
        return str(-80), str(-100)
    latitude, longitude = IPGeoPlugin.latLong(city_country)
    return latitude, longitude

def coordinateDiff(lat1, long1, lat2, long2):
    # Find distance between two coordinates
    # Haversine formula
    # Pre: latitude and longitude for two points in str
    # Post: Distance in km between the two points, float format
    R = 6371
    lat1 = float(lat1)
    lat2 = float(lat2)
    long1 = float(long1)
    long2 = float(long2)
    d_lat = math.radians((lat2-lat1))
    d_long = math.radians((long2-long1))
    lat1 = math.radians(lat1)
    lat2 = math.radians(lat2)
    a = math.sin(d_lat/2)**2 + math.sin(d_long/2)**2 * math.cos(lat1) * math.cos(lat2)
    c = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))
    distance = R * c
    return distance

def IPDistance(IP1, IP2, con):
    # Given two IP addresses, find distance between these in real life
    # Pre: Two IP addresses in str format
    # Post: Distance in km between the two IP addresses in float
    lat1 = ""
    lat2 = ""
    long1 = ""
    long2 = ""
    with con:
        cur = con.cursor()
        cur.execute("SELECT EXISTS(SELECT * FROM IPtoCoord WHERE IP=? LIMIT 1)", [IP1])
        test = cur.fetchone()[0] 
        if int(test) == int(1):
            print "Fetch from cache 1"
            cur.execute('SELECT Lat FROM IPtoCoord WHERE IP=?', [IP1])
            lat1 = str(cur.fetchone()[0])
            cur.execute('SELECT Long FROM IPtoCoord WHERE IP=?', [IP1])
            long1 = str(cur.fetchone()[0])
        else:
            print "Fetch from Geody 1"
            lat1, long1 = IPGeoPlugin.IPGeolocate(IP1)
            cur.execute('INSERT INTO IPtoCoord VALUES(?,?,?)', (IP1,lat1,long1))
        cur.execute('SELECT EXISTS(SELECT * FROM IPtoCoord WHERE IP=? LIMIT 1)', [IP2])
        test = cur.fetchone()[0]
        if int(test) == int(1):
            print "Fetch from cache 1"
            cur.execute('SELECT Lat FROM IPtoCoord WHERE IP=?', [IP2])
            lat2 = (cur.fetchone()[0])
            cur.execute('SELECT Long FROM IPtoCoord WHERE IP=?', [IP2])
            long2 = (cur.fetchone()[0])
        else:
            print "Fetch from Geody 2"
            lat2, long2 = IPGeoPlugin.IPGeolocate(IP2)
            cur.execute('INSERT INTO IPtoCoord VALUES(?,?,?)', (IP2,lat2,long2))
    distance = IPGeoPlugin.coordinateDiff(lat1, long1, lat2, long2)        
    return distance

def initialize():
    con = lite.connect('ip_cache.db')
    with con:
        cur = con.cursor()
        cur.execute('CREATE TABLE IF NOT EXISTS IPtoCoord (IP TEXT, Lat TEXT, Long TEXT)')    

#!/usr/bin/python
# -*- coding: UTF-8 -*- 

import re
import MySQLdb


def get_date(str):
    month = month_to_value(str[0:3])
    pattern = re.compile(r'\d+')
    result = pattern.findall(str)
    date = int(result[1])*1000000 + month * 10000 + int(result[0])*100 + 18
    return date

def get_value(str):
    pattern = re.compile(r'\-*\d+(?:\.\d+)?')
    result = pattern.findall(str)
    value = float(result[2])
    return value
    
def month_to_value(str):
    if str == "Jan":
        return 1
    elif str == "Feb":
        return 2
    elif str == "Mar":
        return 3
    elif str == "Apr":
        return 4
    elif str == "May":
        return 5
    elif str == "Jun":
        return 6
    elif str == "Jul":
        return 7
    elif str == "Aug":
        return 8
    elif str == "Sep":
        return 9
    elif str == "Oct":
        return 10
    elif str == "Nov":
        return 11
    elif str == "Dec":
        return 12

name = raw_input("input:")
file_name = "./stat/%s.txt" % name

fd = open(file_name, "r")
tbl_name = name
rl = []

buf = fd.readline()
while len(buf) > 0 :
    rl.append(buf)
    buf = fd.readline()

fd.close()

db = MySQLdb.connect("localhost","root","ct","ct_db" )
cursor = db.cursor()

for entry in rl :
    date = get_date(entry)
    value = get_value(entry)
    sql = "INSERT INTO %s VALUE('%u', '%f')" % (tbl_name, date, value)
    print sql
    try:
       cursor.execute(sql)
       db.commit()
    except:
       # Rollback in case there is any error
       db.rollback()

db.close()

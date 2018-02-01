#!/usr/bin/python2.7 
import MySQLdb
import datetime

class DB_ENTRY:
    'db entry'
    def __init__(self):
        self.time = 0
        self.date = 0
        self.value = 0.0
        
class DB_TBL:
    'db table'
    def __init__(self,tbl_name):
        self.tbl_name = tbl_name
    def get_last_one(self):
        entry = DB_ENTRY()
        db_con = MySQLdb.connect("localhost","root","ct","ct_db" )
        db_cursor = db_con.cursor()
        try:
            sql = "SELECT * FROM " + self.tbl_name +" ORDER BY time DESC LIMIT 1"
            db_cursor.execute(sql)
            results = db_cursor.fetchall()
            for row in results:
                entry.time = int(row[0])
                entry.value = float(row[1])
                entry.date = int(row[0]/100)
        except:
            entry = None
        db_con.close()
        return entry
        
    def get_all_data_by_date(self):
        # the newest data is in the db_list first place
        data_list = []
        db_con = MySQLdb.connect("localhost","root","ct","ct_db" )
        db_cursor = db_con.cursor()
        try:
            sql = "SELECT FLOOR(time/100) date,avg(value) FROM " + self.tbl_name +" GROUP BY date ORDER BY date DESC"
            db_cursor.execute(sql)
            results = db_cursor.fetchall()
            for row in results:
                entry = DB_ENTRY()
                entry.date = int(row[0])
                entry.value = float(row[1])
                data_list.append(entry)
        except:
            data_list = None
        db_con.close()
        return data_list
        
    def get_oldest_date(self):
        db_con = MySQLdb.connect("localhost","root","ct","ct_db" )
        db_cursor = db_con.cursor()
        date = 0
        try:
            sql = "SELECT FLOOR(time/100) date FROM" + self.tbl_name +" ORDER BY date ASC LIMIT 1"
            db_cursor.execute(sql)
            results = db_cursor.fetchall()
            for row in results:
                date = int(row[0])
        except:
            date = 0
        db_con.close()
        return date
        
    def get_value_by_date(self, date, offset):
        db_con = MySQLdb.connect("localhost","root","ct","ct_db" )
        db_cursor = db_con.cursor()
        value = 0
        if offset != 0:
            date = datetime_offset_date(date, offset)
            
        try:
            sql = "SELECT FLOOR(time/100) date,avg(value)FROM %s WHERE FLOOR(time/100)=%d" % (self.tbl_name, date) 
            db_cursor.execute(sql)
            results = db_cursor.fetchall()
            for row in results:
                value = float(row[1])
        except:
            value = 0
        db_con.close()
        return value

def cal_diff_percent_str(value, cmp_value):
    if cmp_value == 0 :
        return "*"
    #per_diff = (value - cmp_value)/((value + cmp_value)/2)
    per_diff = float((value - cmp_value)/cmp_value)
    str = "%.2f%%" % (per_diff*100)
    return str

def cal_percent_str(value, cmp_value):
    if cmp_value == 0 :
        return "*"
    str = "%.2f%%" % (float(value/cmp_value *100))
    return str

    
def cal_low_list_percent_str(db_list, cmp_value, cmp_lenth):
    if (cmp_lenth > len(db_list)) or (cmp_lenth == 0):
        cmp_lenth = len(db_list)
    sum_num = 0
    low_num = 0
    for entry in db_list:
        if sum_num >= cmp_lenth :
            break
        if cmp_value < entry.value :
            low_num += 1  
        sum_num += 1
    str = "%.2f%%" % (float(100*float(low_num)/sum_num))
    return str

def datetime_offset_date(date, off_date):
    now = datetime.date((date/10000)%10000, (date/100)%100 ,date%100)
    offset = datetime.timedelta(days=off_date)
    new_date = now + offset;
    new_int_date = new_date.year*10000 + new_date.month*100 + new_date.day
    return new_int_date
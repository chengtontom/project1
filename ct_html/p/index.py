#!/usr/bin/python2.7 
import cgi, cgitb, real_time

def html_head() :
    print"Content-Type: text/html"
    print""
    print '''
<html>
    <head>
        <meta charset=\"utf-8\">
        <title>ct-statistics</title>
    </head>
    <body>
'''
def html_tail() :
    print '''</body>
</html>
'''
def html_select_bar() :
    print '''
        <form action="/p/index.py" method="post" target="_self">
        <input type="radio" name="type" value="real-time" /> real-time
        <input type="submit" value="submit" />
        </form>
'''

form = cgi.FieldStorage()
type = "null"
if form.getvalue('type'):
    type = form.getvalue('type')

html_head()
html_select_bar()
if type == 'real-time' :
    real_time.show(0)
if type == 'real-time-color' :
    real_time.show(1)
html_tail()

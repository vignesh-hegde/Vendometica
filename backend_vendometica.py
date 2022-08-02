import sqlite3
from flask import Flask, request
import requests, json
from datetime import datetime
from pytz import timezone
#import razorpay


app = Flask(__name__)



bot_token = "<TOKEN of Telegram Bot>"
chat_id = "<Chat ID of User>"
database = "<Database Name>"
csv = "<CSV File Name>"
serialnum = "<Machine Serial Number>"

#endpoints
refund = "</REFUND_ENDPOINT>"
webhook = "/WEBHOOK_ENDPOINT"
retrive = "</RETRIVE_ENDPOINT>"




# CSV Creation for details 
def create_csv():
    dispen = {'1': "Dispenced","-1":"Refund Issued","0":"Under Process"}
    
    conn = sqlite3.connect(database,check_same_thread=False)
    cursor = conn.cursor()
    sql = '''SELECT * FROM COFFEE_DETAILS;'''
    cursor.execute(sql)
    data = cursor.fetchall()

    try:
        file = open("Details.csv","w")
        file.write("Payment Id, Status, Amount, Time\n")
        if len(data) < 1:
            file.write("Total Amount = Rs. 0.00")
        else:
            amount = 0
            
            for i in data:
                price = int(i[1]) * int(i[2])
                
                if price  > 0:
                    amount += price

                file.write(f"{i[0]}, {dispen[i[1]]}, Rs.{price}.00, {i[3]} IST\n")
            file.write(f"Total Amount = Rs. {amount}.00")
        file.close()
        return 0
    except:
        return 1
    finally:
        conn.commit()
        conn.close()

def send_csv():
    f = open(csv,"rb")
    data = {"document":f}
    requests.post(f"https://api.telegram.org/bot{bot_token}/sendDocument?chat_id={chat_id}",files = data)
    f.close()
    return 0

# Telegram webhook
@app.route('/telegram', methods=['POST'])
def telegram():
    if request.method == 'POST':
        
        my_info = json.loads(request.data)
        
        if my_info['message']['from']["id"] == int(chat_id) and my_info['message']['text'] == "/details":
            create_csv()
            send_csv()
        return "0"
    return "Error"

# Refund coffee
@app.route(refund, methods=['GET'])
def refund_payments():
    if request.method == 'GET':

        conn = sqlite3.connect(database,check_same_thread=False)

        ind_time = datetime.now(timezone("Asia/Kolkata")).strftime('%H:%M')

        data = f"""Dear user,
                    There were issues with the coffee vending machine with SerialNumber: {serialnum}, at {ind_time} IST, 
                    Please visit it and rectify the problem immediately.
                    Thank you for using Vendometica"""

        msg = "https://api.telegram.org/bot{}/sendMessage?chat_id={}&text={}".format(bot_token,chat_id,data)

        requests.get(msg)
        id = request.args.get('id')
        cursor = conn.cursor()
        sql = f'''UPDATE COFFEE_DETAILS SET is_dispensed = "-1", Time = "{ind_time}" WHERE payment_id  = "{id}";'''
        cursor.execute(sql)
        conn.commit()
        conn.close()
        # Razorpay Refund API

        return "0"

    return "GET METHODS ONLY"

#webhook for coffee and sugarcane
@app.route(webhook, methods=['POST'])
def webhook():
    if request.method == 'POST':
        my_info = json.loads(request.data)
        if my_info["event"] == 'payment.captured':
            amount = my_info['payload']['payment']['entity']['amount']
            payment_id = my_info["payload"]["payment"]["entity"]['id']

            if int(amount) == 100 or int(amount) == 200:
                conn = sqlite3.connect(database,check_same_thread=False)
                cur = conn.cursor()
                ind_time = datetime.now(timezone("Asia/Kolkata")).strftime('%H:%M')
                cur.execute(f"INSERT INTO COFFEE_DETAILS (payment_id, is_dispensed,amount, Time) values('{payment_id}','0','{int(amount)//100}','{ind_time}');")
                conn.commit()
                cur.close()
                conn.close()
        return "Webhook received!"
    else:
        return "Invalid Webhook received!"

#retrive for coffee
@app.route(retrive, methods=['GET'])
def retrive():
    if request.method == 'GET':
        conn = sqlite3.connect(database,check_same_thread=False)
        cursor = conn.cursor()
        sql = '''SELECT payment_id,amount FROM COFFEE_DETAILS WHERE is_dispensed=0 LIMIT 1;'''
        cursor.execute(sql)
        records = cursor.fetchall()
        if len(records)==0:
            conn.commit()
            conn.close()
            return "0"
        amount = records[0][1]
        records = records[0][0]
        conn.commit()
        sql = f'''UPDATE COFFEE_DETAILS SET is_dispensed = "1"  WHERE payment_id = "{records}";'''
        cursor.execute(sql)
        conn.commit()
        conn.close()
        return "1"+records+amount
    return "illigal"

# To Create  database
'''
@app.route('/create_database', methods=['GET'])
def create_coffee_database():
    if request.method == 'GET':
        conn = sqlite3.connect(database,check_same_thread=False)
        cursor = connv.cursor()
        sql = """CREATE TABLE COFFEE_DETAILS (
            payment_id TEXT,
            is_dispensed TEXT,
            amount TEXT,
            Time TEXT
            );"""
        cursor.execute(sql)
        conn.commit()
        conn.close()
        return "Database Created"
    return "GET METHODS ONLY"
'''

if __name__ == "__main__":
    app.run()
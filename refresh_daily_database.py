import sqlite3
from backend_vendometica import create_csv,send_csv

bot_token = "<TOKEN of Telegram Bot>"
chat_id = "<Chat ID of User>"
database = "<Database Name>"
csv = "<CSV File Name>"

def delete():
    conn = sqlite3.connect(database,check_same_thread=False)
    cursor = conn.cursor()
    sql = '''DELETE FROM COFFEE_DETAILS;'''
    cursor.execute(sql)
    conn.commit()
    conn.close()
    return 0



if __name__ == "__main__":
    create_csv()
    send_csv()
    delete()
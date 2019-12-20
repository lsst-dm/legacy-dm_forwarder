import os
import pika 

rmq_user = os.environ['RMQ_USR']
rmq_pwd = os.environ['RMQ_PWD']

con = pika.BlockingConnection(pika.URLParameters(f'amqp://{rmq_user}:{rmq_pwd}@141.142.238.15:5672/%2ftest_at'))
ch = con.channel()

def on(ch, method, properties, body):
    print(body)

#ch.basic_consume(on, 'telemetry_queue')
ch.basic_consume(on, 'archive_ctrl_consume')
ch.start_consuming()

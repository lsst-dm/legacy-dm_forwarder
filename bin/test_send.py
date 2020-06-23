import pika
import os
import yaml

rmq_user = os.environ['RMQ_USR']
rmq_pwd = os.environ['RMQ_PWD']

con = pika.BlockingConnection(pika.URLParameters(f'amqp://{rmq_user}:{rmq_pwd}@141.142.238.15:5672/%2ftest_cc'))
ch = con.channel()

image_id = 'gen1'

msg = {}
msg["MSG_TYPE"] = "ASSOCIATED"
msg["ASSOCIATION_KEY"] = "atarchiver_association"
msg["REPLY_QUEUE"] = "atarchiver_ack_publish"
msg["ACK_ID"] = "ack_id"
ch.basic_publish('message', 'f99_consume', yaml.dump(msg))

xfer = {}
xfer["RAFT_LIST"] = "00"
xfer["RAFT_CCD_LIST"] = [ "22/0", "22/1", "22/2" ]
xfer["AT_FWDR"] = "f99"

msg = {}
msg["MSG_TYPE"] = "AT_FWDR_XFER_PARAMS"
msg["REPLY_QUEUE"] = "at_foreman_ack_publish"
msg["IMAGE_ID"] = image_id
msg["ACK_ID"] = "ack_100"
msg["TARGET_LOCATION"] = "ARC@141.142.238.15:/tmp"
msg["SESSION_ID"] = "Session_100"
msg["JOB_NUM"] = "job_100"
msg["XFER_PARAMS"] = xfer
msg["LOCATIONS"] = [ "22/0", "22/1", "22/2" ]
ch.basic_publish('message', 'f99_consume', yaml.dump(msg))

msg = {}
msg['MSG_TYPE'] = 'AT_FWDR_HEADER_READY'
msg['ACK_ID'] = 0
msg['FILENAME'] = f'http://localhost:8000/{image_id}.header'
msg['IMAGE_ID'] = image_id
msg['REPLY_QUEUE'] = 'at_foreman_ack_publish'
ch.basic_publish('message', 'f99_consume', yaml.dump(msg))

msg = {}
msg['MSG_TYPE'] = 'AT_FWDR_END_READOUT'
msg['REPLY_QUEUE'] = 'at_foreman_ack_publish'
msg['IMAGE_ID'] = image_id
msg['ACK_ID'] = 'ack_100'
ch.basic_publish('message', 'f99_consume', yaml.dump(msg))

msg = {}
msg['MSG_TYPE'] = 'SCAN'
msg['DAY'] = 0
ch.basic_publish('message', 'f99_consume', yaml.dump(msg))

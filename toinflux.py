from influxdb import InfluxDBClient

def updatedata(timeline,num,value):
    host='47.88.160.8'
    port=8086
    user = 'root'
    password = 'root'
    dbname = 'unicenter'
    json_body = [
        {
            "measurement": "gas_info",
            "tags": {
                "sensor_no": num,
            },
            "time": timeline,
            "fields": {
                "value": value
            }
        }
    ]

    client = InfluxDBClient(host, port, user, password, dbname)
    print json_body
    client.write_points(json_body)

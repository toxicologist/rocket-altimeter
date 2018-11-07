import serial, time
import plotly.offline as py
import plotly.graph_objs as go

ser = serial.Serial('COM3', 9600, timeout=1, write_timeout=1)

def read_lines():
    ser.write("send".encode())
    d = ser.readlines()
    return d[1:]

def get_values(d):
    processed = []
    for val in d:
        val = str(val)
        val = val.replace("b'","").replace("\\r","").replace("\\n'","")
        val = float(val)/4
        processed.append(val)
    return processed

def filter_values(altitude):
    filtered = []
    for i in range(3,497):
        filtered.append((altitude[i-2]+altitude[i-1]+altitude[i]+altitude[i+1]+altitude[i+2])/5)
    return filtered

def get_apogee(processed):
    apo = max(processed)
    return apo, processed.index(apo)*0.05

def create_graph(processed, apo, title):
    time_values = []
    for i in range(len(processed)):
        time_values.append(i*0.05)

    trace = go.Scatter(
        x = time_values,
        y = processed
    )
    data = [trace]

    layout = go.Layout(
        showlegend = False,
        annotations = [
            dict(
                x=apo[1],
                y=apo[0],
                xref='x',
                yref='y',
                text='Apogee: %fm'%apo[0],
                showarrow=True,
                arrowhead=7,
                ax=0,
                ay=-40
            )
        ]
    )
    
    fig = go.Figure(data=data, layout=layout)
    py.plot(fig, filename='%s.html' % title)

def get_data(title):
    d = read_lines()
    processed = get_values(d)
    print(processed)
    filtered = filter_values(processed)
    apo = get_apogee(filtered)
    print(apo)
    create_graph(filtered,apo,title)

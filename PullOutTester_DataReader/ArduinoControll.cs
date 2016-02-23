using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO.Ports;
using System.Threading;
using System.Windows.Forms;

namespace PullOutTester_DataReader
{
    class ArduinoControll
    {
        SerialPort mySerialPort;
        byte[] dataToWrite = new byte[2];
        string receivedData_String;
        string[] receiveData_StringArray;

        public ArduinoControll(SerialPort mySerialport)
        {
            this.mySerialPort = mySerialport;
        }

        public string[] AskDataFromArduino(int inputData1,int inputData2)
        {
            //intData[2]={16,127}   Check port for Arduino
            //intData[2]={16,128}   Ask Force data from Arduino
            //intData[2]={16,126}   Ask Resistance data from Arduino
            dataToWrite[0] = Convert.ToByte(inputData1);
            dataToWrite[1] = Convert.ToByte(inputData2);
            try
            {
                mySerialPort.Open();
                Thread.Sleep(200);

                //do
                // {
                mySerialPort.Write(dataToWrite, 0, 2);
                Thread.Sleep(2000);
                // } while ((receivedData_String=mySerialPort.ReadExisting()) == "");

                receivedData_String = mySerialPort.ReadExisting();

                // Thread.Sleep(1000);

                //receivedData_String = mySerialPort.ReadExisting();
                Thread.Sleep(500);
                receiveData_StringArray = receivedData_String.Split(new Char[] { ',' });
                mySerialPort.Close();

                return receiveData_StringArray;
            }

            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                string[] error={ex.ToString(), "-1"};
                return error;
            }

            
        }
    }
}

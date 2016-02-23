using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.IO.Ports;
using System.Windows.Forms.DataVisualization.Charting;
using System.Threading;

namespace PullOutTester_DataReader
{
    public partial class Form1 : Form
    {
        SerialPort mySerialPort;
        ArduinoControll arduino;
        string[] ForceDataString;  //voltage value array from MCU
        double[] ForceData;        //  * calibrating factor
        string[] ResistanceDataString;
        double[] ResistanceData;


        public Form1()
        {
            InitializeComponent();
            FillPortCombobox();
            label4.Text = "@Julian Chu" + "\r\n" + "Version 0.00";
        }

        ///Fill port combobox with available COM Ports
        private void FillPortCombobox()
        {
            PortCombobox.Items.Clear();
            SerialPort tmp;

            //scan all available ports and put into combobox
            foreach (string portname in SerialPort.GetPortNames())
            {
                if (PortCombobox.Items.Contains(portname))
                    continue;
                tmp = new SerialPort(portname);

                try
                {
                    tmp.Open();
                    if (tmp.IsOpen)
                    {
                        tmp.Close();
                        PortCombobox.Items.Add(portname);
                    }
                }
                catch (Exception e)
                {
                    MessageBox.Show(e.Message);
                }
            }
        }

        private void GetData_Click(object sender, EventArgs e)
        {
            GetData.Enabled = false;
            save.Enabled = false;
            string PortExist=PortCombobox.Text==""?"":PortCombobox.Text;
            //create new threading to transfer data from MCU to PC
            ThreadPool.QueueUserWorkItem(new WaitCallback(GetData_Thread), PortExist);
        }

        private void GetData_Thread(object PortExist)
        {           
            this.Invoke(
                new UpdateUserInterface(updateUserInterface),
                new object[] { 1 });

            if ((string)PortExist !="")
            {
                mySerialPort = new SerialPort((string)PortExist, 9600, Parity.None, 8, StopBits.Two);
                arduino = new ArduinoControll(mySerialPort);
                int ReadCounts = 0;
                string[] Data;
                do
                {
                    this.Invoke(new UpdateUserInterface(updateUserInterface),
                    new object[] { 3 });                    

                    Data = arduino.AskDataFromArduino(16, 127);
                    ReadCounts++;
                    
                    if (ReadCounts > 10)
                        break;
                } while (Data[0] != "Connected\r\n");

                if (Data[0] == "Connected\r\n")
                {
                    this.Invoke(new UpdateUserInterface(updateUserInterface),
                    new object[] { 4 });
                    ///Read Force data
                   // ForceDataString = null;
                    ForceData = new double[100];
                    ForceDataString = arduino.AskDataFromArduino(16, 128);

                    for(int j=0;j<ForceDataString.Length;j++)
                    {
                        //ForceData[j] = double.Parse(ForceDataString[j])*9/22;  //calibration changed on 15.04.2015
                        ForceData[j] = double.Parse(ForceDataString[j])*25.47/88;  // on 24.08.2015
                    }
                    
                    foreach (double data in ForceData)
                        this.Invoke((MethodInvoker)delegate {
                            var doubleString = data.ToString("f3");
                           
                            ReceivedData.Items.Add(doubleString); });
                        


                    ///Read Resistance data
                    ResistanceData = new double[100];
                    ResistanceDataString = arduino.AskDataFromArduino(16, 126);

                    for (int j = 0; j < ResistanceDataString.Length; j++)
                        ResistanceData[j] = double.Parse(ResistanceDataString[j]);

                    foreach(string data in ResistanceDataString)
                        this.Invoke((MethodInvoker)delegate{R_ReceivedData.Items.Add(data);});

                    // MakeChart(ForceData,ResistanceData);
                    this.Invoke((MethodInvoker)delegate { MakeChart(ForceData, ResistanceData); });

                    this.Invoke(new UpdateUserInterface(updateUserInterface),
                    new object[] { 5 });
                }
                else
                {
                    MessageBox.Show("Can't connect to Arduino! Please choose a port for Arduino");
                    this.Invoke(new UpdateUserInterface(updateUserInterface),
                    new object[] { 2 });
                }
            }
            else
            {
                MessageBox.Show("Please choose a port for Arduino");
                this.Invoke(new UpdateUserInterface(updateUserInterface),
                new object[] { 2 });
            }
        
        }

        // Update the status of the data transfer process
        public delegate void UpdateUserInterface(int Case);
        private void updateUserInterface(int Case)
        {
            switch (Case)
            { 
                case 1:
                    ReceivedData.Items.Clear();
                    break;
                case 2:
                    GetData.Enabled = true;
                    save.Enabled = true;
                    Status.Text = "";
                    break;
                case 3:
                    Status.Text = "";
                    Thread.Sleep(100);
                    Status.Text = "Connecting to Arduino...";
                    break;
                case 4:
                   Status.Text = "Reading data from Arduino...";
                    break;
                case 5:
                   Status.Text = "";
                   GetData.Enabled = true;
                   save.Enabled = true;
                    break;            
            }
        }

        private void MakeChart(double[] Data1,double[] Data2)
        {
            ForceChart.Series.Clear();
            ForceChart.Series.Add("Series1:");

            for (int i = 0; i < Data1.Length - 1; i++)
            {
                ForceChart.Series[0].Points.AddXY(i * 10, Data1[i]);
            }
            ForceChart.Series[0].ChartType = SeriesChartType.Line;

            ///chart2
            chart2.Series.Clear();
            chart2.Series.Add("Series2:");
            for (int i = 0; i < Data2.Length - 1; i++)
            {
                chart2.Series[0].Points.AddXY(i * 10, Data2[i]);
            }
            chart2.Series[0].ChartType = SeriesChartType.Line;

        }

        private void save_Click(object sender, EventArgs e)
        {
            if (ForceData==null)
            {
                MessageBox.Show("No Data to save!","Error",MessageBoxButtons.OK,MessageBoxIcon.Exclamation);
                ErrLog Err = new ErrLog();
                Err.CreateLogFile(Directory.GetCurrentDirectory().ToString(), "No Data to save!");
                //Err.CreateLogFile(@"d:\\", "No Data to save!");
                return;
            }
            SaveFileDialog saveFileDialog1 = new SaveFileDialog();
            saveFileDialog1.InitialDirectory = @"d:\\";
            saveFileDialog1.Filter = "Comma-Delimited Files(*.csv)|*.csv|All Files(*.*)|*.*";
            if(saveFileDialog1.ShowDialog()==DialogResult.OK)
            {
                string name = saveFileDialog1.FileName;
                try
                {
                    using (StreamWriter writer = new StreamWriter(name))
                    {
                        writer.Write("Step,Value,Stroke(um),Force(g),R(ohm)");
                        writer.Write("\n");
                        int i = 0;

                        for (i = 0; i < ForceDataString.Length; i++)
                        {
                            writer.Write(i+","+ForceDataString[i]+","+i*0.02+","+ForceData[i]+","+ResistanceData[i] );
                            writer.Write("\n");
                        }
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show(ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    ErrLog Err = new ErrLog();
                    Err.CreateLogFile(Directory.GetCurrentDirectory().ToString(), ex.Message.ToString());
                }
            }
        }

        private void PortCombobox_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

    }
}

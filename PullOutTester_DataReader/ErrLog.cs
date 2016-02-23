using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;

namespace PullOutTester_DataReader
{
    class ErrLog
    {
        private string sLogFormat;
        private string sErrTime;

        public ErrLog()
        {
            //yyyy/mm/dd hh:mm:ss AM/PM ==> Log Message
            sLogFormat = DateTime.Now.ToShortDateString().ToString() + " " + DateTime.Now.ToLongTimeString().ToString() + " Error: ";
            string sYear = DateTime.Now.Year.ToString();
            string sMonth = DateTime.Now.Month.ToString();
            string sDay = DateTime.Now.Day.ToString();
            sErrTime=sYear+sMonth+sDay;
        }

        public void  CreateLogFile(string sPathName, string sErrMsg)
        {
            string sFileName=sPathName+@"\Errlog.txt";
            /*
             * if (!File.Exists(sFileName))
            {
                using (File.Create(sFileName)) ;
                
            }
             */
            using (StreamWriter sw = File.AppendText(sFileName))
            {
                sw.WriteLine(sLogFormat + sErrMsg);
                sw.Flush();
                sw.Close();
            };
        }
    }
}

using System;
using System.Diagnostics;
using System.Threading;
using System.Windows.Forms;

namespace AndroidBDDControl
{
    public partial class Form1 : Form
    {
        delegate void AppendLogCallback (string logString);

        public Form1()
        {
            InitializeComponent();
        }

        private void runProcessAndPrintLog (string cmd, string args, bool closeImmediately = false)
        {
            appendLog ("========================================================================="
                       + "\r\n => " + cmd + " " + args + "\r\n");
            Process p = new Process();
            // Redirect the output stream of the child process.
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.CreateNoWindow = true;
            p.StartInfo.RedirectStandardOutput = true;
            p.StartInfo.FileName = cmd;
            p.StartInfo.Arguments = args;
            p.OutputDataReceived += new DataReceivedEventHandler ( (sender, e) =>
            {
                appendLog (e.Data);
            });

            p.Start();
            p.BeginOutputReadLine();

            if (closeImmediately)
            {
                p.Close();
            }
            else
            {
                while (!p.WaitForExit (200))
                {
                    Thread.Yield();
                }
            }
        }

        private void appendLog (string logString)
        {
            if (logString == null)
            {
                return;
            }

            if (textBoxLog.InvokeRequired)
            {
                Invoke (new AppendLogCallback (appendLog), new object[] { logString });
            }
            else
            {
                if (!logString.EndsWith ("\r\n"))
                {
                    logString += "\r\n";
                }
                textBoxLog.Text += logString;
                textBoxLog.SelectionStart = textBoxLog.TextLength;
                textBoxLog.ScrollToCaret();
            }
        }

        private void btnCheckAdb_Click (object sender, EventArgs e)
        {
            runProcessAndPrintLog ("adb", "devices");
        }

        private void btnStartApp_Click (object sender, EventArgs e)
        {
            runProcessAndPrintLog ("adb", "push libs /storage/sdcard0/bdd");
            runProcessAndPrintLog ("adb", "install -r AndroidBddRunner.apk");
            runProcessAndPrintLog ("adb", "logcat -c");
            runProcessAndPrintLog ("adb", "shell am start -n com.royale.royalebdd/com.royale.royalebdd.MainActivity");
            runProcessAndPrintLog ("adb", "logcat -s ROYALE_BDD", true);
        }

        private void btnResult_Click (object sender, EventArgs e)
        {
            runProcessAndPrintLog ("adb", "pull /storage/sdcard0/bdd/royale_bdd_report.txt .");
            appendLog ("done");
        }

        private void btnEnableTCP_Click (object sender, EventArgs e)
        {
            runProcessAndPrintLog ("adb", "tcpip 5555");
            appendLog ("done, now connect the Camera to the tablet.");
        }

        private void btnConnect_Click (object sender, EventArgs e)
        {
            runProcessAndPrintLog ("adb", "connect " + textBoxIP.Text + ":5555");
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace polyv_player_demo_csharp
{
    public class LogWrite
    {
        public LogWrite()
        {
            //
            // TODO: 在此处添加构造函数逻辑
            //
        }

        //单个日志文件大小
        private const long FILESIZE = 10 * 1024 * 1024;
        //日志文件数
        private const int LOGFILENUM = 5;
        //日志类型
        private const short DEBUG = 0;
        private const short INFO = 1;
        private const short WARNING = 2;
        private const short ERROR = 3;
        //日志级别
        private const short LOGLEVEL = DEBUG;

        //日志文件路径
        private string m_LogDir = System.AppDomain.CurrentDomain.BaseDirectory + "Logs\\";
        private string m_filePath = System.AppDomain.CurrentDomain.BaseDirectory + "Logs\\" + "appLog_" + DateTime.Now.ToString("yyyy-MM-dd") + ".txt";//工作日志文件

        private static LogWrite m_Log = null;
        private Object objLock = new Object();
        private string destClass = string.Empty;


        /// <summary>
        /// 记录日志的类名
        /// </summary>
        public string DestClass
        {
            get { return destClass; }
            set { destClass = value; }
        }

        /// <summary>
        /// 创建日志类实例
        /// </summary>
        /// <returns></returns>
        public static LogWrite CreateInstance()
        {
            if (m_Log == null)
            {
                m_Log = new LogWrite();
            }
            return m_Log;
        }

        public bool SetPath(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                return false;
            }
            try
            {
                if (!Directory.Exists(path))
                {
                    Directory.CreateDirectory(path);
                }
            }
            catch(Exception)
            {
                return false;
            }
            m_LogDir = path;
            m_filePath = path + "\\appLog_" + DateTime.Now.ToString("yyyy-MM-dd") + ".txt";
            return true;
        }

        /// <summary>
        /// 写信息日志
        /// </summary>
        /// <param name="content"></param>
        public void Info(string content)
        {
            if (LOGLEVEL <= 1)
            {
                lock (objLock)
                {
                    WriteLog(content, LogWrite.INFO);
                }
            }
        }

        /// <summary>
        /// 写调试日志
        /// </summary>
        /// <param name="content"></param>
        public void Debug(string content)
        {
            if (LOGLEVEL <= 0)
            {
                lock (objLock)
                {
                    WriteLog(content, LogWrite.DEBUG);
                }
            }
        }

        /// <summary>
        /// 写警告日志
        /// </summary>
        /// <param name="content"></param>
        public void Warning(string content)
        {
            if (LOGLEVEL <= 2)
            {
                lock (objLock)
                {
                    WriteLog(content, LogWrite.WARNING);
                }
            }
        }

        /// <summary>
        /// 写错误日志
        /// </summary>
        /// <param name="content"></param>
        public void Error(string content)
        {
            if (LOGLEVEL <= 3)
            {
                lock (objLock)
                {
                    WriteLog(content, LogWrite.ERROR);
                }
            }
        }

        /// <summary>
        /// 写日志
        /// </summary>
        /// <param name="content"></param>
        /// <param name="type"></param>
        private void WriteLog(string content, short type)
        {
            if (content == null || content.Equals(string.Empty))
            {
                return;
            }
            try
            {
                if (!File.Exists(m_filePath))
                {
                    CreateWorkLogFile();
                }

                if (LogFileIsTooLarge(m_filePath))
                {
                    ProcessLogFile();
                }

                string strType = "";
                switch (type)
                {
                    case DEBUG:
                        strType = "[DEBUG]";
                        break;
                    case INFO:
                        strType = "[INFO]";
                        break;
                    case WARNING:
                        strType = "[WARNING]";
                        break;
                    case ERROR:
                        strType = "[ERROR]";
                        break;
                    default:
                        strType = "[INFO]";
                        break;
                }

                string strContent = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss  ") + strType + "  " + content;
                StreamWriter sWriter = new StreamWriter(m_filePath, true, System.Text.Encoding.UTF8);
                sWriter.WriteLine(strContent);
                sWriter.Flush();
                sWriter.Close();
                sWriter = null;

                Console.WriteLine(strContent);
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine(ex.Message);
            }
        }
        /// <summary>
        /// 创建工作日志文件
        /// </summary>
        private void CreateWorkLogFile()
        {
            if (!Directory.Exists(m_LogDir))
            {
                Directory.CreateDirectory(m_LogDir);
            }
            FileStream fStream = new FileStream(m_filePath, FileMode.CreateNew);
            fStream.Flush();
            fStream.Close();
            fStream = null;
        }


        /// <summary>
        /// 判断日志文件是否超过预定义最大值
        /// </summary>
        /// <param name="fileName"></param>
        /// <returns></returns>
        private bool LogFileIsTooLarge(string fileName)
        {
            FileInfo fi = new FileInfo(fileName);
            if (fi.Length >= FILESIZE)
            {
                return true;
            }
            else
            {
                return false;
            }
        }


        /// <summary>
        /// 处理日志文件（工作日志文件超过最大值）
        /// </summary>
        private void ProcessLogFile()
        {
            try
            {
                DirectoryInfo dirInfo = new DirectoryInfo(m_LogDir);
                FileInfo[] vFileInfo = dirInfo.GetFiles("*.txt");
                if (vFileInfo != null && vFileInfo.Length > LOGFILENUM)
                {
                    FileInfo oldFile = vFileInfo[0];
                    foreach (FileInfo fi in vFileInfo)
                    {
                        if (DateTime.Compare(fi.LastWriteTime, oldFile.LastWriteTime) < 0)
                        {
                            oldFile = fi;
                        }
                    }
                    File.Delete(oldFile.FullName);
                }
                File.Copy(m_filePath, m_LogDir + "\\" + DateTime.Now.ToString("yyyyMMddHHmmss") + ".txt");
                File.Delete(m_filePath);
                CreateWorkLogFile();
            }
            catch
            {

            }
        }
    }
}

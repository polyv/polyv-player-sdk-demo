using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Net;
using System.Web.Script.Serialization;
using System.Security.Cryptography;

namespace polyv_player_demo_csharp
{
    public class VideoTokenInfo
    {
        public string Token { get; set; }
        public string UserId { get; set; }
        public string AppId { get; set; }
        public string VideoId { get; set; }
        public string ViewerIp { get; set; }
        public string ViewerId { get; set; }
        public string ViewerName { get; set; }
        public string ExtraParams { get; set; }
        public long Ttl { get; set; }
        public long CreatedTime { get; set; }
        public long ExpiredTime { get; set; }
        public int Iswxa { get; set; }
        public bool Disposable { get; set; }
    }

    public class VideoTokenResponse
    {
        public int Code { get; set; }
        public string Status { get; set; }
        public string Message { get; set; }
        public VideoTokenInfo Data { get; set; }
    }

    public class VideoInfo
    {
        public string Title { get; set; }
        public string Vid { get; set; }
    }

    public class VideoListResponse
    {
        public int Code { get; set; }
        public string Status { get; set; }
        public string Message { get; set; }
        public List<VideoInfo> Data;
    }

    class PlayerUtilities
    {
        public static string millSecond2Time(long millisecs)
        {
            TimeSpan s = new TimeSpan(millisecs * 10000);
            return string.Format("{0:D2}:{1:D2}:{2:D2}", s.Hours, s.Minutes, s.Seconds);
        }

        public static string GenerateMD5(string str, bool upper)
        {
            byte[] byteArray = Encoding.UTF8.GetBytes(str);
            byteArray = MD5CryptoServiceProvider.Create().ComputeHash(byteArray);
            string hashValue = "";
            foreach (byte b in byteArray)
            {
                hashValue += b.ToString("x2");
            }
            if (upper)
            {
                return hashValue.ToUpper();
            }
            else
            {
                return hashValue;
            }
        }

        public static string GenerateSHA1(string str, bool upper)
        {
            byte[] byteArray = Encoding.UTF8.GetBytes(str);
            byteArray = SHA1.Create().ComputeHash(byteArray);
            string hashValue = "";
            foreach (byte b in byteArray)
            {
                hashValue += b.ToString("x2");
            }
            if (upper)
            {
                return hashValue.ToUpper();
            }
            else
            {
                return hashValue;
            }
        }

        public static string Get(string url, Dictionary<string, string> pms)
        {
            string result = "";
            StringBuilder builder = new StringBuilder();
            builder.Append(url);
            if (pms.Count > 0)
            {
                builder.Append("?");
                int i = 0;
                foreach (var item in pms)
                {
                    if (i > 0)
                        builder.Append("&");
                    builder.AppendFormat("{0}={1}", item.Key, item.Value);
                    i++;
                }
            }
            try
            {
                HttpWebRequest req = (HttpWebRequest)WebRequest.Create(builder.ToString());
                req.ContentType = "application/x-www-form-urlencoded;charset=UTF-8";
                HttpWebResponse resp = (HttpWebResponse)req.GetResponse();
                Stream stream = resp.GetResponseStream();
                try
                {
                    using (StreamReader reader = new StreamReader(stream))
                    {
                        result = reader.ReadToEnd();
                    }
                }
                finally
                {
                    stream.Close();
                }
            }
            catch (Exception e)
            {
                result = e.Message;
            }
            return result;
        }

        public static string Post(string url, Dictionary<string, string> pms)
        {
            string result = "";
            HttpWebRequest req = (HttpWebRequest)WebRequest.Create(url);
            req.Method = "POST";
            req.ContentType = "application/x-www-form-urlencoded";
            
            StringBuilder builder = new StringBuilder();
            int i = 0;
            foreach (var item in pms)
            {
                if (i > 0)
                    builder.Append("&");
                builder.AppendFormat("{0}={1}", item.Key, item.Value);
                i++;
            }
            byte[] data = Encoding.UTF8.GetBytes(builder.ToString());
            req.ContentLength = data.Length;
            using (Stream reqStream = req.GetRequestStream())
            {
                reqStream.Write(data, 0, data.Length);
                reqStream.Close();
            }
            try
            {
                HttpWebResponse resp = (HttpWebResponse)req.GetResponse();
                Stream stream = resp.GetResponseStream();
            
                using (StreamReader reader = new StreamReader(stream, Encoding.UTF8))
                {
                    result = reader.ReadToEnd();
                }
            }
            catch(Exception e)
            {
                result = e.Message;
            }
            return result;
        }

        public static bool RequestVideoList(ref List<VideoInfo> vlist, ref string msg, string userId, string secretKey)
        {
            string url = String.Format("https://api.polyv.net/v2/video/{0}/search", userId);
            string ts = ((long)(DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, 0)).TotalMilliseconds).ToString();
            string sign = "ptime=" + ts +
                "&userid=" + userId + secretKey;
            sign = GenerateSHA1(sign, true);

            var pms = new Dictionary<string, string>();
            pms.Add("ptime", ts);
            pms.Add("userid", userId);
            pms.Add("sign", sign);

            //js
            try
            {
                string res = Get(url, pms);
                JavaScriptSerializer serializer = new JavaScriptSerializer();
                VideoListResponse vl = new VideoListResponse();
                vl = serializer.Deserialize<VideoListResponse>(res);
                if (vl.Code == 200)
                {
                    vlist = vl.Data;
                    msg = vl.Status + ":" + vl.Message;
                    return true;
                }
                else
                {
                    msg = vl.Status + ":" + vl.Message;
                    return false;
                }
            }catch(Exception e)
            {
                msg = e.Message;
                return false;
            }
        }

        public static bool GetToken(ref string token, string userId, string secertKey, string vid, string viewerId, string viewerName)
        {
            string url = "http://hls.videocc.net/service/v1/token";
            string ts = ((long)(DateTime.UtcNow - new DateTime(1970, 1, 1, 0, 0, 0, 0)).TotalMilliseconds).ToString();
            string sign = secertKey +
                "extraParams" + "pc-sdk" +
                "ts" + ts +
                "userId" + userId +
                "videoId" + vid;
            if (viewerId != string.Empty && viewerName != string.Empty)
            {
                sign = sign +
                    "viewerId" + viewerId +
                    "viewerName" + viewerName;
            }
            sign = sign + secertKey;
            sign = GenerateMD5(sign, true);

            var postdata = new Dictionary<string, string>();
            postdata.Add("extraParams", "pc-sdk");
            postdata.Add("ts", ts);
            postdata.Add("userId", userId);
            postdata.Add("videoId", vid);
            if (viewerId != string.Empty && viewerName != string.Empty)
            {
                postdata.Add("viewerId", viewerId);
                postdata.Add("viewerName", viewerName);
            }
            postdata.Add("sign", sign);
            try
            {
                string res = Post(url, postdata);
                JavaScriptSerializer serializer = new JavaScriptSerializer();
                VideoTokenResponse vif = new VideoTokenResponse();
                vif = serializer.Deserialize<VideoTokenResponse>(res);
                if (vif.Code == 200)
                {
                    token = vif.Data.Token;
                    return true;
                }
                else
                {
                    token = res;
                    return false;
                }
            }
            catch (Exception e)
            {
                token = e.Message;
                return false;
            }
        }
    }
}
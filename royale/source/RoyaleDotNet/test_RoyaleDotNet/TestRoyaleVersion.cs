using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using RoyaleDotNet;

namespace test_RoyaleDotNet
{
    [TestClass]
    public class TestRoyaleVersion
    {
        [TestMethod]
        public void TestVersionsMatch ()
        {
            UInt32 major, minor, patch, build;
            String scm;
            CameraStatus status = Royale.GetVersion (out major, out minor, out patch, out build, out scm);

            Version version = typeof (RoyaleDotNet.CameraDevice).Assembly.GetName().Version;

            Assert.AreEqual (CameraStatus.SUCCESS, status);
            Assert.AreEqual ( (UInt32) version.Major, major);
            Assert.AreEqual ( (UInt32) version.Minor, minor);
            Assert.AreEqual ( (UInt32) version.Build, patch);
            Assert.AreEqual ( (UInt32) version.Revision, build);
            Assert.IsNotNull (scm);
        }
    }
}

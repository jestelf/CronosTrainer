using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Imaging;
using System.IO;

static class IconGen {
 static readonly Color Back=Color.FromArgb(16,14,13),Ring=Color.FromArgb(214,128,58),Glass=Color.FromArgb(232,214,180),Sand=Color.FromArgb(240,170,80);
 static Bitmap Draw(int s){
  var bmp=new Bitmap(s,s,PixelFormat.Format32bppArgb);
  using(var g=Graphics.FromImage(bmp)){
   g.SmoothingMode=SmoothingMode.AntiAlias;g.Clear(Color.Transparent);
   float r=s*0.18f;var box=new RectangleF(s*0.02f,s*0.02f,s*0.96f,s*0.96f);
   using(var path=Rounded(box,r))using(var bg=new LinearGradientBrush(box,Color.FromArgb(40,32,28),Back,90f))g.FillPath(bg,path);
   float w=Math.Max(1.5f,s*0.06f);var ring=new RectangleF(s*0.14f,s*0.14f,s*0.72f,s*0.72f);
   using(var pen=new Pen(Ring,w))g.DrawEllipse(pen,ring);
   if(s>=32)using(var tick=new Pen(Ring,Math.Max(1f,s*0.025f))){float cx=s/2f,cy=s/2f,ro=s*0.36f,ri=s*0.31f;
    for(int i=0;i<12;i++){double a=i*Math.PI/6;g.DrawLine(tick,cx+(float)Math.Cos(a)*ri,cy+(float)Math.Sin(a)*ri,cx+(float)Math.Cos(a)*ro,cy+(float)Math.Sin(a)*ro);}}

   float L=s*0.37f,Rr=s*0.63f,T=s*0.29f,B=s*0.71f,M=s*0.5f,n=s*0.035f;
   using(var pen=new Pen(Glass,Math.Max(1.2f,s*0.045f)){LineJoin=LineJoin.Round,StartCap=LineCap.Round,EndCap=LineCap.Round}){
    g.DrawLine(pen,L-s*0.03f,T,Rr+s*0.03f,T);g.DrawLine(pen,L-s*0.03f,B,Rr+s*0.03f,B);
    g.DrawLines(pen,new[]{new PointF(L,T),new PointF(M-n,M),new PointF(L,B)});g.DrawLines(pen,new[]{new PointF(Rr,T),new PointF(M+n,M),new PointF(Rr,B)});}
   using(var br=new SolidBrush(Sand)){
    g.FillPolygon(br,new[]{new PointF(L+s*0.06f,T+s*0.08f),new PointF(Rr-s*0.06f,T+s*0.08f),new PointF(M,M-s*0.02f)});
    g.FillPolygon(br,new[]{new PointF(M,B-s*0.12f),new PointF(Rr-s*0.03f,B-s*0.02f),new PointF(L+s*0.03f,B-s*0.02f)});}
  }
  return bmp;
 }
 static GraphicsPath Rounded(RectangleF b,float r){var p=new GraphicsPath();float d=r*2;p.AddArc(b.X,b.Y,d,d,180,90);p.AddArc(b.Right-d,b.Y,d,d,270,90);p.AddArc(b.Right-d,b.Bottom-d,d,d,0,90);p.AddArc(b.X,b.Bottom-d,d,d,90,90);p.CloseFigure();return p;}
 static void Main(string[] args){
  string dir=args[0];int[] sizes={16,20,24,32,40,48,64,128,256};var images=new byte[sizes.Length][];
  for(int i=0;i<sizes.Length;i++)using(var b=Draw(sizes[i]))using(var ms=new MemoryStream()){b.Save(ms,ImageFormat.Png);images[i]=ms.ToArray();if(sizes[i]==256)File.WriteAllBytes(Path.Combine(dir,"cronos.png"),images[i]);}
  using(var f=new BinaryWriter(File.Create(Path.Combine(dir,"cronos.ico")))){
   f.Write((short)0);f.Write((short)1);f.Write((short)sizes.Length);int offset=6+16*sizes.Length;
   for(int i=0;i<sizes.Length;i++){f.Write((byte)(sizes[i]>=256?0:sizes[i]));f.Write((byte)(sizes[i]>=256?0:sizes[i]));f.Write((byte)0);f.Write((byte)0);f.Write((short)1);f.Write((short)32);f.Write(images[i].Length);f.Write(offset);offset+=images[i].Length;}
   foreach(var img in images)f.Write(img);
  }
 }
}

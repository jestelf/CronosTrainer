using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Media;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
[assembly: AssemblyTitle("Cronos Trainer")]
[assembly: AssemblyProduct("Cronos Trainer")]
[assembly: AssemblyVersion("1.0.0.0")]

static class L {
 public static bool En;
 static readonly Dictionary<string,string> Map=new Dictionary<string,string>{
  {"Бессмертие","God mode"},{"Здоровье держится на максимуме, смертельный удар не убивает","Health stays at maximum, a lethal blow does not kill"},
  {"Неуязвимость (no hit)","Invulnerable (no hit)"},{"Удары, взрывы, огонь и кислота не задевают героя","Blows, explosions, fire and acid do not touch the hero"},
  {"Убийство с одного удара","One-hit kills"},{"Любое попадание по врагу забирает всё его здоровье","Any hit takes all of the enemy's health"},
  {"Бесконечные патроны","Infinite ammo"},{"Штатный режим игры UnlimitedAmmo: магазин не расходуется","The game's own UnlimitedAmmo mode: the magazine is not used up"},
  {"Без перезарядки","No reload"},{"Магазин всегда полный, перезаряжать не нужно","The magazine is always full, no need to reload"},
  {"Бесконечная энергия костюма","Infinite suit energy"},{"Энергия костюма и деления всегда полные","Suit energy and its bars are always full"},
  {"Полёт сквозь стены (no clip)","Fly through walls (no clip)"},{"WASD — полёт по взгляду, пробел/Ctrl — вверх/вниз, Shift — быстрее","WASD flies where you look, Space/Ctrl up/down, Shift faster"},
  {"Урон оружия ×N","Weapon damage ×N"},{"Бонус урона всех стволов и ближнего боя (множитель ниже)","Damage bonus for every gun and melee (multiplier below)"},
  {"Без отдачи","No recoil"},{"Отдача обычного и заряженного выстрела снята","Recoil of normal and charged shots is removed"},
  {"Идеальная точность","Perfect accuracy"},{"Без разброса после выстрела и раскачки прицела","No spread after shots and no aim sway"},
  {"Мгновенная перезарядка","Instant reload"},{"Анимация перезарядки ×5 быстрее","Reload animation ×5 faster"},
  {"Скорострельность ×4","Fire rate ×4"},{"Задержки между выстрелами всех стволов вчетверо короче","Delays between shots of every gun are four times shorter"},
  {"Мгновенный заряд","Instant charge"},{"Заряженный выстрел готов сразу","The charged shot is ready at once"},
  {"Бесконечное топливо и мины","Infinite fuel and mines"},{"Запас топлива огнемёта и мин не убывает","Flamethrower fuel and mines never run out"},
  {"Расширенный инвентарь","Bigger inventory"},{"+12 ячеек инвентаря","+12 inventory slots"},
  {"Бесплатный крафт","Free crafting"},{"Ресурсы для фабрикатора всегда на максимуме","Fabricator resources are always at maximum"},
  {"Быстрый бег ×N","Fast running ×N"},{"Множитель скорости передвижения героя (ниже)","Hero movement speed multiplier (below)"},
  {"Подсказка кода замка","Lock code hint"},{"На кодовой панели всегда виден правильный код","The keypad always shows the correct code"},
  {"Враги не нападают","Enemies do not attack"},{"Монстры кружат рядом, но не подходят и не атакуют","Monsters circle nearby but do not close in or attack"},
  {"ПЕРЕКЛЮЧАТЕЛЕЙ","TOGGLES"},{"Ctrl + F12 — отключить все читы","Ctrl + F12 — turn every cheat off"},{"○ Игра не подключена","○ Game not connected"},
  {"Запустите Cronos: The New Dawn и загрузите сохранение","Start Cronos: The New Dawn and load a save"},{"Отключить всё  ·  Ctrl+F12","Turn all off  ·  Ctrl+F12"},
  {"Множитель урона","Damage multiplier"},{"Скорость бега","Running speed"},{"Меняются на лету, в том числе при включённой функции","Change on the fly, even while the function is on"},
  {"Разовые действия — выданное остаётся в сохранении","One-time actions — what you get stays in the save"},
  {"Полное здоровье","Full health"},{"Здоровье восстановлено","Health restored"},{"Боеприпасы","Ammo"},{"Выданы боеприпасы","Ammo given"},
  {"Все ресурсы","All resources"},{"Выданы все ресурсы","All resources given"},{"Все предметы","All items"},{"Выданы все предметы","All items given"},
  {"Всё снаряжение","All equipment"},{"Выдано всё снаряжение","All equipment given"},{"Все улучшения","All upgrades"},{"Открыты все улучшения","All upgrades unlocked"},
  {"Убить врагов рядом","Kill nearby enemies"},{"Враги рядом уничтожены","Nearby enemies destroyed"},{"Души…","Souls…"},
  {"+1 000 Энергии","+1,000 Energy"},{"Добавлено 1 000 Энергии","Added 1,000 Energy"},{"+10 000 Энергии","+10,000 Energy"},{"Добавлено 10 000 Энергии","Added 10,000 Energy"},
  {"+1 Ядро","+1 Core"},{"Добавлено 1 Ядро","Added 1 Core"},{"+5 Ядер","+5 Cores"},{"Добавлено 5 Ядер","Added 5 Cores"},
  {"Ресурсы крафта до максимума","Crafting resources to max"},{"Ресурсы фабрикатора пополнены","Fabricator resources refilled"},
  {"Журнал","Log"},{"Справка","Help"},{"Звук переключений","Toggle sounds"},{"Горячие клавиши","Hotkeys"},
  {"Игра не запущена","Game is not running"},{"Запустите Cronos: The New Dawn. Трейнер подключится автоматически.","Start Cronos: The New Dawn. The trainer connects automatically."},
  {"Подключение…","Connecting…"},{"Активно: ","Active: "},{" из "," of "},{"● Подключено · PID ","● Connected · PID "},
  {"Восстанавливаю значения игры…","Restoring game values…"},{"Трейнер закрыт · значения игры восстановлены","Trainer closed · game values restored"},
  {"Игра закрыта · ожидание запуска","Game closed · waiting for launch"},{"Игра закрыта — читы будут включены снова при следующем подключении","Game closed — cheats will be turned back on at the next connection"},
  {"Подключено к Cronos · PID ","Connected to Cronos · PID "},{" · мост загружен"," · bridge loaded"},{"Не включено снова: ","Not turned back on: "},{"Повторно включено: ","Turned back on: "},
  {"Связь: ","Link: "},{"Связь с игрой потеряна · переподключение","Lost the link to the game · reconnecting"},{"Игра не подключена","Game not connected"},
  {"включено","on"},{"выключено","off"},{"Не выполнено: ","Failed: "},{"Все функции уже выключены","Every function is already off"},{"Все функции выключены","Every function is off"},
  {"Отключение: ","Turning off: "},{"Для этого действия нужна запущенная игра","This action needs the running game"},{"Множитель урона: ×","Damage multiplier: ×"},{"Скорость бега: ×","Running speed: ×"},
  {"Параметр: ","Parameter: "},{"Запуск Cronos Trainer · PID ","Cronos Trainer started · PID "},{"Горячая клавиша занята: ","Hotkey is taken: "},{"Ctrl + F12 занята другим приложением","Ctrl + F12 is taken by another application"},
  {"без клавиши","no key"},{" (занята)"," (taken)"},{"Язык: русский","Language: English"},{"ВКЛ","ON"},{"ВЫКЛ","OFF"},
  {"Cronos Trainer · души","Cronos Trainer · souls"},{"Души из таблицы SoulsData игры · добавленная душа остаётся в сохранении","Souls from the game's SoulsData table · an added soul stays in the save"},
  {"Добавить душу","Add soul"},{"Убрать душу","Remove soul"},{"Добавлена душа: ","Soul added: "},{"Убрана душа: ","Soul removed: "},{"Души: ","Souls: "},
  {"Таблица душ не загружена — откройте окно после загрузки сохранения","The soul table is not loaded — open this window after loading a save"},{"Душ в таблице: ","Souls in the table: "},
  {" · добавленная душа остаётся в сохранении"," · an added soul stays in the save"},{"Cronos Trainer · журнал","Cronos Trainer · log"},{"Cronos Trainer · справка","Cronos Trainer · help"},
  {"Cronos Trainer уже запущен.","Cronos Trainer is already running."},{"Cronos Trainer: ошибка","Cronos Trainer: error"},
  {"Игра загружается…","Game is loading…"},{"Не удалось загрузить мост в игру","Could not load the bridge into the game"},
  {"Нет доступа к процессу игры (запустите трейнер от имени администратора)","No access to the game process (run the trainer as administrator)"},
  {"Запущено несколько копий игры — оставьте одну","Several copies of the game are running — keep one"},
  {"Мост закрыл канал","The bridge closed the channel"},{"Мост отключён","Bridge disconnected"},{"Чтение памяти игры не выполнено","Reading game memory failed"}
 };
 static readonly string[][] Frag={
  new[]{"Другая версия игры (","Different game version ("},new[]{") — трейнер рассчитан на GOG-сборку 1.4.0.0",") — the trainer targets the GOG build 1.4.0.0"},
  new[]{"Мост не отвечает: ","Bridge does not respond: "},new[]{"Ошибка подключения: ","Connection error: "},
  new[]{"перехват не установлен (другая версия игры)","hook not installed (different game version)"},new[]{"игра не ответила (пауза или загрузка)","the game did not answer (pause or loading)"},
  new[]{"сбой при выполнении (команда отменена)","execution failed (command cancelled)"},new[]{"нет игрока (загрузите сохранение)","no player (load a save)"},
  new[]{"нет игрока","no player"},new[]{"нет компонента здоровья","no health component"},new[]{"максимум здоровья не получен","maximum health not available"},
  new[]{"нет такой души: ","no such soul: "},new[]{"нет предмета ","no item "},new[]{"неизвестная функция ","unknown function "},new[]{"неизвестная команда","unknown command"},
  new[]{"операция","operation"},new[]{"значение","value"},new[]{"параметр","parameter"},new[]{"исключение","exception"}
 };
 public static string T(string s){if(!En || s==null)return s;string e;return Map.TryGetValue(s,out e)?e:s;}
 public static string D(string s){if(!En || String.IsNullOrEmpty(s))return s;string e;if(Map.TryGetValue(s,out e))return e;foreach(var f in Frag)s=s.Replace(f[0],f[1]);return s;}
}

static class Skin {
 public static readonly Color BG=Color.FromArgb(17,15,14),Panel=Color.FromArgb(31,27,25),Text=Color.FromArgb(240,234,226),Muted=Color.FromArgb(160,150,140),
  Accent=Color.FromArgb(232,146,70),Amber=Color.FromArgb(236,190,90),Off=Color.FromArgb(56,50,46),Red=Color.FromArgb(232,104,96);
 public static Label Label(string text,int size=10,Color? color=null,bool bold=false){return new Label{Text=text,ForeColor=color??Text,Font=new Font("Segoe UI",size,bold?FontStyle.Bold:FontStyle.Regular),AutoSize=false,BackColor=Color.Transparent,TextAlign=ContentAlignment.MiddleLeft};}
 public static Button Button(string text,Action action,bool primary=false){var b=new Button{Text=text,FlatStyle=FlatStyle.Flat,BackColor=primary?Accent:Panel,ForeColor=primary?BG:Text,Font=new Font("Segoe UI",9.5f,FontStyle.Bold),Height=35,Cursor=Cursors.Hand,TabStop=true};b.FlatAppearance.BorderSize=0;b.FlatAppearance.MouseOverBackColor=primary?Color.FromArgb(244,170,100):Color.FromArgb(46,40,37);b.Click+=delegate{action();};return b;}
 public static void Form(Form f,string title,Size size){f.Text=title;f.ClientSize=size;f.BackColor=BG;f.ForeColor=Text;f.Font=new Font("Segoe UI",10);f.StartPosition=FormStartPosition.CenterParent;f.AutoScaleMode=AutoScaleMode.Dpi;}
}

static class Sounds {
 static SoundPlayer on,off;
 public static void Init(){on=new SoundPlayer(new MemoryStream(Wave(new[]{620.0,930.0})));off=new SoundPlayer(new MemoryStream(Wave(new[]{930.0,560.0})));on.Load();off.Load();}
 public static void Play(bool enabled){try{(enabled?on:off).Play();}catch{}}
 static byte[] Wave(double[] tones){
  const int rate=44100;int seg=rate*75/1000;int n=seg*tones.Length;var pcm=new short[n];
  for(int t=0;t<tones.Length;t++)for(int i=0;i<seg;i++){double env=Math.Min(1.0,i/(rate*0.006))*Math.Min(1.0,(seg-i)/(rate*0.018));double s=Math.Sin(2*Math.PI*tones[t]*i/rate)*0.75+Math.Sin(4*Math.PI*tones[t]*i/rate)*0.12;pcm[t*seg+i]=(short)(s*env*0.32*short.MaxValue);}
  var ms=new MemoryStream();var w=new BinaryWriter(ms);w.Write(Encoding.ASCII.GetBytes("RIFF"));w.Write(36+n*2);w.Write(Encoding.ASCII.GetBytes("WAVEfmt "));w.Write(16);w.Write((short)1);w.Write((short)1);w.Write(rate);w.Write(rate*2);w.Write((short)2);w.Write((short)16);w.Write(Encoding.ASCII.GetBytes("data"));w.Write(n*2);foreach(short s in pcm)w.Write(s);return ms.ToArray();
 }
}

sealed class Cheat {public string Key,Title,Description;public bool Active,Pending;}

sealed class CheatRow : Control {
 public readonly Cheat Cheat;public readonly int Index;public string Hotkey="";public bool Available;
 public CheatRow(int index,Cheat cheat){Index=index;Cheat=cheat;SetStyle(ControlStyles.AllPaintingInWmPaint|ControlStyles.UserPaint|ControlStyles.OptimizedDoubleBuffer|ControlStyles.Selectable,true);BackColor=Skin.Panel;Height=58;Margin=new Padding(4);Cursor=Cursors.Hand;TabStop=true;AccessibleName=L.T(cheat.Title);AccessibleRole=AccessibleRole.CheckButton;}
 protected override void OnPaint(PaintEventArgs e){
  var g=e.Graphics;g.Clear(BackColor);
  if(Cheat.Active)using(var b=new SolidBrush(Skin.Accent))g.FillRectangle(b,0,0,3,Height);
  using(var f=new Font("Segoe UI",10,FontStyle.Bold))TextRenderer.DrawText(g,L.T(Cheat.Title),f,new Rectangle(14,7,Width-95,23),Available?Skin.Text:Skin.Muted,TextFormatFlags.EndEllipsis);
  using(var f=new Font("Segoe UI",8))TextRenderer.DrawText(g,(Hotkey!=""?Hotkey+"  ·  ":"")+L.T(Cheat.Description),f,new Rectangle(14,32,Width-90,21),Skin.Muted,TextFormatFlags.EndEllipsis);
  Rectangle box=new Rectangle(Width-66,14,52,28);using(var b=new SolidBrush(Cheat.Pending?Skin.Amber:Cheat.Active?Skin.Accent:Skin.Off))g.FillRectangle(b,box);
  using(var f=new Font("Segoe UI",9,FontStyle.Bold))TextRenderer.DrawText(g,Cheat.Pending?"…":L.T(Cheat.Active?"ВКЛ":"ВЫКЛ"),f,box,Cheat.Active||Cheat.Pending?Skin.BG:Skin.Muted,TextFormatFlags.HorizontalCenter|TextFormatFlags.VerticalCenter);
  if(Focused)ControlPaint.DrawFocusRectangle(g,new Rectangle(2,2,Width-4,Height-4),Skin.Accent,BackColor);
 }
 protected override void OnMouseEnter(EventArgs e){BackColor=Color.FromArgb(40,35,32);base.OnMouseEnter(e);}
 protected override void OnMouseLeave(EventArgs e){BackColor=Skin.Panel;base.OnMouseLeave(e);}
 protected override void OnClick(EventArgs e){if(Available && !Cheat.Pending)base.OnClick(e);}
 protected override void OnKeyDown(KeyEventArgs e){if(e.KeyCode==Keys.Space || e.KeyCode==Keys.Enter){OnClick(EventArgs.Empty);e.Handled=true;}base.OnKeyDown(e);}
}

sealed class Trainer : Form {
 static readonly string User=Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),"CronosTrainer");
 public static readonly Cheat[] Cheats={
  new Cheat{Key="god",Title="Бессмертие",Description="Здоровье держится на максимуме, смертельный удар не убивает"},
  new Cheat{Key="nohit",Title="Неуязвимость (no hit)",Description="Удары, взрывы, огонь и кислота не задевают героя"},
  new Cheat{Key="onehit",Title="Убийство с одного удара",Description="Любое попадание по врагу забирает всё его здоровье"},
  new Cheat{Key="ammo",Title="Бесконечные патроны",Description="Штатный режим игры UnlimitedAmmo: магазин не расходуется"},
  new Cheat{Key="noreload",Title="Без перезарядки",Description="Магазин всегда полный, перезаряжать не нужно"},
  new Cheat{Key="energy",Title="Бесконечная энергия костюма",Description="Энергия костюма и деления всегда полные"},
  new Cheat{Key="noclip",Title="Полёт сквозь стены (no clip)",Description="WASD — полёт по взгляду, пробел/Ctrl — вверх/вниз, Shift — быстрее"},
  new Cheat{Key="damage",Title="Урон оружия ×N",Description="Бонус урона всех стволов и ближнего боя (множитель ниже)"},
  new Cheat{Key="norecoil",Title="Без отдачи",Description="Отдача обычного и заряженного выстрела снята"},
  new Cheat{Key="accuracy",Title="Идеальная точность",Description="Без разброса после выстрела и раскачки прицела"},
  new Cheat{Key="reload",Title="Мгновенная перезарядка",Description="Анимация перезарядки ×5 быстрее"},
  new Cheat{Key="firerate",Title="Скорострельность ×4",Description="Задержки между выстрелами всех стволов вчетверо короче"},
  new Cheat{Key="charge",Title="Мгновенный заряд",Description="Заряженный выстрел готов сразу"},
  new Cheat{Key="explosives",Title="Бесконечное топливо и мины",Description="Запас топлива огнемёта и мин не убывает"},
  new Cheat{Key="inventory",Title="Расширенный инвентарь",Description="+12 ячеек инвентаря"},
  new Cheat{Key="fabrication",Title="Бесплатный крафт",Description="Ресурсы для фабрикатора всегда на максимуме"},
  new Cheat{Key="speed",Title="Быстрый бег ×N",Description="Множитель скорости передвижения героя (ниже)"},
  new Cheat{Key="autocode",Title="Подсказка кода замка",Description="На кодовой панели всегда виден правильный код"},
  new Cheat{Key="noattack",Title="Враги не нападают",Description="Монстры кружат рядом, но не подходят и не атакуют"}
 };
 public static readonly float[] DamageSteps={2,3,5,10,25}, SpeedSteps={1.5f,2,2.5f,3};
 const int GridRows=10;public static bool Preview;
 ComboBox damageMul,speedMul;Form souls;Button ruButton,enButton;

 readonly List<CheatRow> rows=new List<CheatRow>();readonly List<Button> actions=new List<Button>();readonly Dictionary<int,Action> hotkeys=new Dictionary<int,Action>();
 readonly System.Windows.Forms.Timer uiTimer=new System.Windows.Forms.Timer{Interval=500};readonly DateTime started=DateTime.Now;readonly List<string> events=new List<string>();
 readonly object sync=new object();CronosLink link;string linkState="Игра не запущена";volatile bool closing,busy;Thread worker;
 Label connection,metrics,lastEvent,runtime;CheckBox sound,keys;TextBox historyText;Form history;

 public Trainer(){
  Skin.Form(this,"Cronos Trainer",new Size(1000,900));MinimumSize=new Size(900,560);StartPosition=FormStartPosition.CenterScreen;AutoScroll=true;
  try{Icon=Icon.ExtractAssociatedIcon(Application.ExecutablePath);}catch{}
  bool snd=true,k=true;LoadSettings(ref snd,ref k);
  Build(2,1,snd,k);Directory.CreateDirectory(User);
  try{string log=Path.Combine(User,"history.log");if(File.Exists(log))events.AddRange(File.ReadAllLines(log,Encoding.UTF8).Reverse().Take(300).Reverse());}catch{}
  Log(L.T("Запуск Cronos Trainer · PID ")+Process.GetCurrentProcess().Id);
  uiTimer.Tick+=delegate{Sync();};uiTimer.Start();
  if(!Preview){worker=new Thread(Loop){IsBackground=true,Name="CronosLink"};worker.Start();}
  FormClosing+=OnClosingTrainer;
 }

 void Build(int dmg,int spd,bool snd,bool k){
  int[] heights={70,62,GridRows*62,48,206,62};
  var layout=new TableLayoutPanel{Dock=DockStyle.Top,Height=heights.Sum()+22,Padding=new Padding(18,12,18,10),ColumnCount=1,RowCount=heights.Length};layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,100));
  foreach(int h in heights)layout.RowStyles.Add(new RowStyle(SizeType.Absolute,h));Controls.Add(layout);
  var head=new Panel{Dock=DockStyle.Fill};
  var image=new PictureBox{Location=new Point(1,2),Size=new Size(58,58),SizeMode=PictureBoxSizeMode.Zoom};
  try{image.Image=Image.FromStream(Assembly.GetExecutingAssembly().GetManifestResourceStream("cronos.png"));}catch{}
  head.Controls.Add(image);
  var title=Skin.Label("CRONOS",24,Skin.Accent,true);title.SetBounds(73,0,320,38);head.Controls.Add(title);
  var sub=Skin.Label("THE NEW DAWN  /  TRAINER  ·  "+Cheats.Length+" "+L.T("ПЕРЕКЛЮЧАТЕЛЕЙ"),9,Skin.Muted);sub.SetBounds(76,37,420,24);head.Controls.Add(sub);
  runtime=Skin.Label("",9,Skin.Muted);runtime.TextAlign=ContentAlignment.MiddleRight;runtime.SetBounds(330,5,405,24);head.Controls.Add(runtime);
  var hk=Skin.Label(L.T("Ctrl + F12 — отключить все читы"),9,Skin.Muted);hk.TextAlign=ContentAlignment.MiddleRight;hk.SetBounds(330,30,405,24);head.Controls.Add(hk);
  ruButton=LangButton("RU",!L.En,delegate{SetLanguage(false);});enButton=LangButton("EN",L.En,delegate{SetLanguage(true);});head.Controls.Add(ruButton);head.Controls.Add(enButton);
  Action place=delegate{runtime.Left=head.Width-520;hk.Left=head.Width-520;ruButton.SetBounds(head.Width-100,8,46,30);enButton.SetBounds(head.Width-50,8,46,30);};
  head.Resize+=delegate{place();};place();layout.Controls.Add(head,0,0);
  var stat=new Panel{Dock=DockStyle.Fill,BackColor=Skin.Panel,Margin=new Padding(4,0,4,6)};connection=Skin.Label(L.T("○ Игра не подключена"),11,Skin.Amber,true);connection.SetBounds(14,4,820,27);stat.Controls.Add(connection);
  metrics=Skin.Label(L.T("Запустите Cronos: The New Dawn и загрузите сохранение"),9,Skin.Muted);metrics.SetBounds(14,30,820,22);stat.Controls.Add(metrics);
  stat.Resize+=delegate{connection.Width=stat.Width-28;metrics.Width=stat.Width-28;};layout.Controls.Add(stat,0,1);
  var grid=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=2,RowCount=GridRows,Margin=new Padding(0)};grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));grid.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,50));
  for(int i=0;i<GridRows;i++)grid.RowStyles.Add(new RowStyle(SizeType.Percent,100f/GridRows));
  for(int i=0;i<Cheats.Length;i++){var row=new CheatRow(i,Cheats[i]){Dock=DockStyle.Fill,Hotkey=HotkeyName(i)};row.Click+=delegate{Toggle(row.Cheat);};rows.Add(row);grid.Controls.Add(row,i/GridRows,i%GridRows);}
  var clearRow=Skin.Button(L.T("Отключить всё  ·  Ctrl+F12"),delegate{ClearAll();});clearRow.Dock=DockStyle.Fill;clearRow.Margin=new Padding(4);clearRow.BackColor=Color.FromArgb(56,30,30);clearRow.ForeColor=Skin.Red;
  grid.Controls.Add(clearRow,Cheats.Length/GridRows,Cheats.Length%GridRows);actions.Add(clearRow);
  layout.Controls.Add(grid,0,2);
  var prm=new Panel{Dock=DockStyle.Fill,BackColor=Skin.Panel,Margin=new Padding(4,2,4,4)};
  var dl=Skin.Label(L.T("Множитель урона"),9,Skin.Muted);dl.SetBounds(14,9,140,24);prm.Controls.Add(dl);
  damageMul=Combo(DamageSteps,dmg);damageMul.SetBounds(156,8,90,26);prm.Controls.Add(damageMul);
  var sl=Skin.Label(L.T("Скорость бега"),9,Skin.Muted);sl.SetBounds(272,9,120,24);prm.Controls.Add(sl);
  speedMul=Combo(SpeedSteps,spd);speedMul.SetBounds(392,8,90,26);prm.Controls.Add(speedMul);
  var pn=Skin.Label(L.T("Меняются на лету, в том числе при включённой функции"),8,Skin.Muted);pn.SetBounds(504,9,420,24);prm.Controls.Add(pn);
  damageMul.SelectedIndexChanged+=delegate{SendParam("damage",DamageSteps[damageMul.SelectedIndex]);};speedMul.SelectedIndexChanged+=delegate{SendParam("speed",SpeedSteps[speedMul.SelectedIndex]);};
  layout.Controls.Add(prm,0,3);
  var act=new TableLayoutPanel{Dock=DockStyle.Fill,ColumnCount=4,RowCount=5,Margin=new Padding(4,3,4,0)};for(int i=0;i<4;i++)act.ColumnStyles.Add(new ColumnStyle(SizeType.Percent,25));
  act.RowStyles.Add(new RowStyle(SizeType.Absolute,22));for(int i=0;i<4;i++)act.RowStyles.Add(new RowStyle(SizeType.Absolute,42));
  var note=Skin.Label(L.T("Разовые действия — выданное остаётся в сохранении"),8,Skin.Muted);note.Dock=DockStyle.Fill;act.Controls.Add(note,0,0);act.SetColumnSpan(note,4);
  AddAction(act,0,1,"Полное здоровье","Здоровье восстановлено","heal");
  AddAction(act,1,1,"Боеприпасы","Выданы боеприпасы","cheat GiveAmmo");
  AddAction(act,2,1,"Все ресурсы","Выданы все ресурсы","cheat GiveAllResources");
  AddAction(act,3,1,"Все предметы","Выданы все предметы","cheat GiveAllItems");
  AddAction(act,0,2,"Всё снаряжение","Выдано всё снаряжение","cheat GiveAllEquipment");
  AddAction(act,1,2,"Все улучшения","Открыты все улучшения","cheat UnlockAllUpgrades");
  AddAction(act,2,2,"Убить врагов рядом","Враги рядом уничтожены","cheat KillAll");
  var sb=Skin.Button(L.T("Души…"),delegate{OpenSouls();},true);Place(act,sb,3,2);actions.Add(sb);
  AddAction(act,0,3,"+1 000 Энергии","Добавлено 1 000 Энергии","give Energy 1000");
  AddAction(act,1,3,"+10 000 Энергии","Добавлено 10 000 Энергии","give Energy 10000");
  AddAction(act,2,3,"+1 Ядро","Добавлено 1 Ядро","give Core 1");
  AddAction(act,3,3,"+5 Ядер","Добавлено 5 Ядер","give Core 5");
  AddAction(act,0,4,"Ресурсы крафта до максимума","Ресурсы фабрикатора пополнены","give Resources 0");
  Place(act,Skin.Button(L.T("Журнал"),delegate{OpenHistory();}),2,4);Place(act,Skin.Button(L.T("Справка"),delegate{OpenHelp();}),3,4);
  layout.Controls.Add(act,0,4);
  var footer=new Panel{Dock=DockStyle.Fill};sound=new CheckBox{Text=L.T("Звук переключений"),Checked=snd,ForeColor=Skin.Muted,AutoSize=true,Location=new Point(6,5)};keys=new CheckBox{Text=L.T("Горячие клавиши"),Checked=k,ForeColor=Skin.Muted,AutoSize=true,Location=new Point(200,5)};footer.Controls.Add(sound);footer.Controls.Add(keys);
  lastEvent=Skin.Label("",8,Skin.Muted);lastEvent.SetBounds(6,29,830,24);footer.Controls.Add(lastEvent);footer.Resize+=delegate{lastEvent.Width=footer.Width-12;};layout.Controls.Add(footer,0,5);
  sound.CheckedChanged+=delegate{SaveSettings();};keys.CheckedChanged+=delegate{SaveSettings();UnregisterKeys();UpdateHotkeys();};
  if(events.Count>0)lastEvent.Text=events[events.Count-1].Length>21?events[events.Count-1].Substring(21):"";
 }
 Button LangButton(string text,bool active,Action action){var b=Skin.Button(text,action,active);b.Font=new Font("Segoe UI",9,FontStyle.Bold);return b;}
 public void SetLanguage(bool en){
  if(L.En==en)return;L.En=en;
  int d=damageMul.SelectedIndex,s=speedMul.SelectedIndex;bool snd=sound.Checked,k=keys.Checked;
  UnregisterKeys();SuspendLayout();var old=Controls.Cast<Control>().ToList();Controls.Clear();foreach(var c in old)c.Dispose();rows.Clear();actions.Clear();
  Build(d,s,snd,k);ResumeLayout(true);SaveSettings();Sync();Log(L.T("Язык: русский"));
 }
 ComboBox Combo(float[] steps,int def){var c=new ComboBox{DropDownStyle=ComboBoxStyle.DropDownList,BackColor=Skin.BG,ForeColor=Skin.Text,FlatStyle=FlatStyle.Flat,Font=new Font("Segoe UI",10)};foreach(float s in steps)c.Items.Add("×"+s.ToString(System.Globalization.CultureInfo.InvariantCulture));
  c.DrawMode=DrawMode.OwnerDrawFixed;c.ItemHeight=22;c.DrawItem+=delegate(object o,DrawItemEventArgs e){bool sel=(e.State&DrawItemState.Selected)!=0 && (e.State&DrawItemState.ComboBoxEdit)==0;using(var b=new SolidBrush(sel?Color.FromArgb(70,52,38):Skin.Panel))e.Graphics.FillRectangle(b,e.Bounds);if(e.Index>=0)TextRenderer.DrawText(e.Graphics,c.Items[e.Index].ToString(),c.Font,e.Bounds,Skin.Accent,TextFormatFlags.Left|TextFormatFlags.VerticalCenter);};
  c.SelectedIndex=Math.Max(0,Math.Min(def,c.Items.Count-1));return c;}
 void SendParam(string name,float v){if(!Live())return;Task.Run(delegate{try{Send("mul "+name+" "+v.ToString(System.Globalization.CultureInfo.InvariantCulture));Post(L.T(name=="damage"?"Множитель урона: ×":"Скорость бега: ×")+v);}catch(Exception e){Post(L.T("Параметр: ")+L.D(e.Message));}});}
 void PushParams(){try{Send("mul damage "+DamageSteps[damageMul.SelectedIndex].ToString(System.Globalization.CultureInfo.InvariantCulture));Send("mul speed "+SpeedSteps[speedMul.SelectedIndex].ToString(System.Globalization.CultureInfo.InvariantCulture));}catch{}}
 void OpenSouls(){
  if(souls!=null && !souls.IsDisposed){souls.Activate();return;}
  souls=new Form();Skin.Form(souls,L.T("Cronos Trainer · души"),new Size(520,560));souls.Icon=Icon;
  var list=new ListBox{BackColor=Skin.Panel,ForeColor=Skin.Text,BorderStyle=BorderStyle.None,IntegralHeight=false,Font=new Font("Segoe UI",10)};list.SetBounds(16,44,488,430);list.Anchor=AnchorStyles.Top|AnchorStyles.Bottom|AnchorStyles.Left|AnchorStyles.Right;
  var note=Skin.Label(L.T("Души из таблицы SoulsData игры · добавленная душа остаётся в сохранении"),8,Skin.Muted);note.SetBounds(16,12,488,24);
  var add=Skin.Button(L.T("Добавить душу"),delegate{var s=list.SelectedItem as string;if(s!=null)Run(L.T("Добавлена душа: ")+s,"soul add "+s);},true);add.SetBounds(16,486,236,38);add.Anchor=AnchorStyles.Bottom|AnchorStyles.Left;
  var rem=Skin.Button(L.T("Убрать душу"),delegate{var s=list.SelectedItem as string;if(s!=null)Run(L.T("Убрана душа: ")+s,"soul remove "+s);});rem.SetBounds(268,486,236,38);rem.Anchor=AnchorStyles.Bottom|AnchorStyles.Right;
  souls.Controls.AddRange(new Control[]{note,list,add,rem});souls.Show(this);
  Task.Run(delegate{try{string r=Send("rows SoulsData");var names=r.StartsWith("ok")?r.Split(new[]{"\n"},StringSplitOptions.RemoveEmptyEntries).Skip(1).ToArray():new string[0];
   souls.BeginInvoke((Action)delegate{list.Items.Clear();foreach(var n in names)list.Items.Add(n);note.Text=names.Length==0?L.T("Таблица душ не загружена — откройте окно после загрузки сохранения"):L.T("Душ в таблице: ")+names.Length+L.T(" · добавленная душа остаётся в сохранении");});}
   catch(Exception e){Post(L.T("Души: ")+L.D(e.Message));}});
 }
 void Place(TableLayoutPanel p,Button b,int x,int y){b.Dock=DockStyle.Fill;b.Margin=new Padding(0,2,6,2);p.Controls.Add(b,x,y);}
 void AddAction(TableLayoutPanel p,int x,int y,string title,string done,string command){var b=Skin.Button(L.T(title),delegate{Run(L.T(done),command);});Place(p,b,x,y);actions.Add(b);}
 bool Live(){CronosLink l=link;return !closing && l!=null && l.Alive;}

 void Loop(){
  DateTime nextAttach=DateTime.MinValue;
  while(!closing){
   try{
    if(link!=null && !link.Alive){lock(sync){try{link.Dispose();}catch{}link=null;}linkState="Игра закрыта · ожидание запуска";Post(L.T("Игра закрыта — читы будут включены снова при следующем подключении"));}
    if(link==null && DateTime.UtcNow>=nextAttach){
     nextAttach=DateTime.UtcNow.AddSeconds(3);string state;var l=CronosLink.Attach(out state);linkState=state;
     if(l!=null){lock(sync)link=l;linkState="ok";Post(L.T("Подключено к Cronos · PID ")+l.Pid+L.T(" · мост загружен"));PushParams();
      var restore=Cheats.Where(c=>c.Active).ToList();
      foreach(var c in restore)try{Send("set "+c.Key+" 1");}catch(Exception e){c.Active=false;Post(L.T("Не включено снова: ")+L.T(c.Title)+" · "+L.D(e.Message));}
      if(restore.Count>0)Post(L.T("Повторно включено: ")+String.Join(", ",restore.Select(c=>L.T(c.Title))));}
    }
   }catch(Exception e){
    Post(L.T("Связь: ")+L.D(e.Message));
    lock(sync){if(link!=null){try{link.Dispose();}catch{}link=null;}}linkState="Связь с игрой потеряна · переподключение";
   }
   Thread.Sleep(200);
  }
 }
 string Send(string command){CronosLink l;lock(sync)l=link;if(l==null)throw new InvalidOperationException("Игра не подключена");return l.Send(command);}

 void Toggle(Cheat c){
  if(!Live() || c.Pending || busy)return;c.Pending=true;Sync();bool target=!c.Active;
  Task.Run(delegate{
   try{string r=Send("set "+c.Key+" "+(target?1:0));if(!r.StartsWith("ok"))throw new InvalidOperationException(r.StartsWith("err ")?r.Substring(4):r);
    c.Active=target;Post(L.T(c.Title)+": "+L.T(target?"включено":"выключено"));BeginInvoke((Action)delegate{Beep(target);});}
   catch(Exception e){Post(L.T("Не выполнено: ")+L.T(c.Title)+" · "+L.D(e.Message));}
   finally{c.Pending=false;}
  });
 }
 void ClearAll(){
  if(!Live() || busy)return;var on=Cheats.Where(c=>c.Active).ToList();if(on.Count==0){Post(L.T("Все функции уже выключены"));return;}busy=true;
  Task.Run(delegate{try{foreach(var c in on){c.Pending=true;try{Send("set "+c.Key+" 0");c.Active=false;}finally{c.Pending=false;}}Post(L.T("Все функции выключены")+" ("+on.Count+")");BeginInvoke((Action)delegate{Beep(false);});}catch(Exception e){Post(L.T("Отключение: ")+L.D(e.Message));}finally{busy=false;}});
 }
 void Run(string title,string command){
  if(!Live()){Post(L.T("Для этого действия нужна запущенная игра"));return;}
  Task.Run(delegate{try{string r=Send(command);if(!r.StartsWith("ok"))throw new InvalidOperationException(r.StartsWith("err ")?r.Substring(4):r);Post(title);BeginInvoke((Action)delegate{Beep(true);});}catch(Exception e){Post(L.T("Не выполнено: ")+L.D(e.Message));}});
 }
 [DllImport("dwmapi.dll")]static extern int DwmSetWindowAttribute(IntPtr h,int attr,ref int value,int size);
 protected override void OnHandleCreated(EventArgs e){base.OnHandleCreated(e);int dark=1;try{DwmSetWindowAttribute(Handle,20,ref dark,4);}catch{}}

 void Sync(){
  if(connection==null || connection.IsDisposed)return;
  bool live=Live();CronosLink l=link;
  connection.Text=live?L.T("● Подключено · PID ")+l.Pid+" · Cronos: The New Dawn 1.4.0.0":"○ "+(linkState=="ok"?L.T("Подключение…"):L.D(linkState));
  connection.ForeColor=live?Skin.Accent:Skin.Amber;
  metrics.Text=live?L.T("Активно: ")+Cheats.Count(c=>c.Active)+L.T(" из ")+Cheats.Length:L.T("Запустите Cronos: The New Dawn. Трейнер подключится автоматически.");
  foreach(var r in rows){r.Available=live && !busy;r.Invalidate();}foreach(var b in actions)b.Enabled=live && !busy;
  runtime.Text=(L.En?"Session ":"Сеанс ")+(DateTime.Now-started).ToString(@"hh\:mm\:ss")+"  ·  PID "+Process.GetCurrentProcess().Id;
  UpdateHotkeys();
 }
 void Beep(bool on){if(sound.Checked)Sounds.Play(on);}
 void Post(string message){try{if(IsHandleCreated && !IsDisposed)BeginInvoke((Action)delegate{Log(message);});}catch{}}
 void Log(string message){string line=DateTime.Now.ToString("dd.MM.yyyy HH:mm:ss")+"  "+message;events.Add(line);if(events.Count>1000)events.RemoveRange(0,events.Count-1000);if(lastEvent!=null && !lastEvent.IsDisposed)lastEvent.Text=message;if(historyText!=null && !historyText.IsDisposed)historyText.AppendText(line+Environment.NewLine);try{File.AppendAllText(Path.Combine(User,"history.log"),line+Environment.NewLine,Encoding.UTF8);}catch{}}
 void LoadSettings(ref bool snd,ref bool k){try{string f=Path.Combine(User,"settings.txt");if(!File.Exists(f))return;foreach(string line in File.ReadAllLines(f)){var p=line.Split('=');if(p.Length!=2)continue;if(p[0]=="sound")snd=p[1]=="1";if(p[0]=="hotkeys")k=p[1]=="1";if(p[0]=="lang")L.En=p[1]=="en";}}catch{}}
 void SaveSettings(){if(Preview)return;try{Directory.CreateDirectory(User);File.WriteAllText(Path.Combine(User,"settings.txt"),"sound="+(sound.Checked?1:0)+"\r\nhotkeys="+(keys.Checked?1:0)+"\r\nlang="+(L.En?"en":"ru")+"\r\n");}catch{}}
 void OpenHistory(){if(history!=null && !history.IsDisposed){history.Activate();return;}history=new Form();Skin.Form(history,L.T("Cronos Trainer · журнал"),new Size(860,500));history.Icon=Icon;historyText=new TextBox{Dock=DockStyle.Fill,ReadOnly=true,Multiline=true,ScrollBars=ScrollBars.Vertical,BackColor=Skin.Panel,ForeColor=Skin.Text,BorderStyle=BorderStyle.None,Font=new Font("Consolas",9.5f),Text=String.Join(Environment.NewLine,events.ToArray())+Environment.NewLine};history.Controls.Add(historyText);history.Show(this);historyText.SelectionStart=historyText.TextLength;historyText.ScrollToCaret();}
 void OpenHelp(){var f=new Form();Skin.Form(f,L.T("Cronos Trainer · справка"),new Size(760,560));f.Icon=Icon;
  var keysText=String.Join("\r\n",rows.Select(r=>"  "+r.Hotkey.PadRight(16)+L.T(r.Cheat.Title)));
  string ru="Cronos Trainer работает с GOG-версией Cronos: The New Dawn 1.4.0.0 (Cronos-Win64-Shipping.exe от 12.02.2026).\r\n\r\n"+
   "1. Запустите игру и загрузите сохранение.\r\n2. Откройте трейнер — он подключится сам (порядок запуска не важен).\r\n\r\n"+
   "Трейнер загружает в игру мост CronosBridge.dll, который выполняет команды в игровом потоке через функции и данные самой игры: "+
   "компонент здоровья героя, флаги MaiHealth врагов, тег UnlimitedAmmo, характеристики GAS (CronosUpgradeSet, CronosSoulPerkSet), параметры режимов стрельбы, "+
   "функцию предметов CollectItem (Энергия, Ядро, ресурсы, топливо, мины) и встроенные функции разработчиков (GiveAllItems, KillAll, AddSoul).\r\n\r\n"+
   "No hit отключает реакцию героя на боевые каналы коллизии, снимает флаг bCanBeDamaged и эффекты горения и кислоты. "+
   "«Враги не нападают» — штатный тег State.Invulnerable. No clip — герой без коллизии и гравитации, движение по взгляду камеры.\r\n"+
   "Выключайте функции характеристик перед сохранением, чтобы в файл попали исходные значения.\r\n\r\n"+
   "Горячие клавиши (работают и в игре):\r\n"+keysText+"\r\n  Ctrl+F12          Отключить всё\r\n\r\n"+
   "При закрытии трейнера все переключатели выключаются, исходные значения возвращаются.\r\n"+
   "Журнал и настройки: %LOCALAPPDATA%\\CronosTrainer";
  string en="Cronos Trainer works with the GOG version of Cronos: The New Dawn 1.4.0.0 (Cronos-Win64-Shipping.exe from 2026-02-12).\r\n\r\n"+
   "1. Start the game and load a save.\r\n2. Open the trainer — it connects by itself (launch order does not matter).\r\n\r\n"+
   "The trainer loads the CronosBridge.dll bridge into the game; it runs commands on the game thread through the game's own functions and data: "+
   "the hero's health component, MaiHealth flags of enemies, the UnlimitedAmmo tag, GAS attributes (CronosUpgradeSet, CronosSoulPerkSet), weapon fire mode parameters, "+
   "the CollectItem item function (Energy, Core, resources, fuel, mines) and built-in developer functions (GiveAllItems, KillAll, AddSoul).\r\n\r\n"+
   "No hit makes the hero ignore the combat collision channels, clears the bCanBeDamaged flag and removes burning and acid effects. "+
   "\"Enemies do not attack\" is the game's own State.Invulnerable tag. No clip removes collision and gravity and moves the hero where the camera looks.\r\n"+
   "Turn attribute functions off before saving so that the original values go into the save file.\r\n\r\n"+
   "Hotkeys (work in game too):\r\n"+keysText+"\r\n  Ctrl+F12          Turn all off\r\n\r\n"+
   "Closing the trainer turns every toggle off and restores the original values.\r\n"+
   "Log and settings: %LOCALAPPDATA%\\CronosTrainer";
  var info=new TextBox{Multiline=true,ReadOnly=true,Dock=DockStyle.Fill,BackColor=Skin.Panel,ForeColor=Skin.Text,BorderStyle=BorderStyle.None,ScrollBars=ScrollBars.Vertical,Font=new Font("Segoe UI",10),Text=L.En?en:ru};
  f.Controls.Add(info);f.Show(this);}

 [DllImport("user32.dll")]static extern bool RegisterHotKey(IntPtr h,int id,uint mods,uint key);
 [DllImport("user32.dll")]static extern bool UnregisterHotKey(IntPtr h,int id);
 void UpdateHotkeys(){
  bool enable=Live() && keys.Checked;if(!enable){if(hotkeys.Count>0){UnregisterKeys();foreach(var r in rows)r.Hotkey=HotkeyName(r.Index);}return;}
  if(hotkeys.Count>0 || !IsHandleCreated)return;
  foreach(var row in rows){int i=row.Index;row.Hotkey=HotkeyName(i);int id=100+i;
   int j=i%10;uint vk=(uint)(j<9?0x61+j:0x60);uint mods=(uint)(0x4000|2|(i>=10?1:0));
   if(RegisterHotKey(Handle,id,mods,vk)){CheatRow capture=row;hotkeys[id]=delegate{Toggle(capture.Cheat);};}else{row.Hotkey+=L.T(" (занята)");Log(L.T("Горячая клавиша занята: ")+L.T(row.Cheat.Title));}}
  if(RegisterHotKey(Handle,200,0x4002,0x7B))hotkeys[200]=delegate{ClearAll();};else Log(L.T("Ctrl + F12 занята другим приложением"));
 }
 static string HotkeyName(int i){if(i>=20)return L.T("без клавиши");int j=i%10;return (i<10?"Ctrl":"Ctrl+Alt")+"+Num"+(j<9?j+1:0);}
 void UnregisterKeys(){if(!IsHandleCreated)return;foreach(int id in hotkeys.Keys)UnregisterHotKey(Handle,id);hotkeys.Clear();}
 protected override void WndProc(ref Message m){if(m.Msg==0x312){Action a;if(hotkeys.TryGetValue(m.WParam.ToInt32(),out a))a();}base.WndProc(ref m);}

 bool shutdownDone;
 void OnClosingTrainer(object sender,FormClosingEventArgs e){
  if(shutdownDone)return;e.Cancel=true;if(closing)return;closing=true;uiTimer.Stop();UnregisterKeys();
  foreach(var r in rows)r.Available=false;foreach(var b in actions)b.Enabled=false;connection.Text=L.T("Восстанавливаю значения игры…");
  Task.Run(delegate{
   try{CronosLink l;lock(sync){l=link;link=null;}if(l!=null){var until=DateTime.UtcNow.AddSeconds(6);if(l.Alive)foreach(var c in Cheats.Where(c=>c.Active)){if(DateTime.UtcNow>until)break;try{string r=l.Send("set "+c.Key+" 0");if(r.StartsWith("err"))break;}catch{break;}}l.Dispose();}}catch{}
   BeginInvoke((Action)delegate{Log(L.T("Трейнер закрыт · значения игры восстановлены"));shutdownDone=true;Close();});
  });
 }

 static int SelfTest(string[] args){
  int i=Array.IndexOf(args,"--selftest");string report=i+1<args.Length?args[i+1]:"selftest.txt";var log=new StringBuilder();
  Action<string> say=s=>{log.AppendLine(DateTime.Now.ToString("HH:mm:ss.fff ")+s);File.WriteAllText(report,log.ToString(),Encoding.UTF8);};
  string state;var link=CronosLink.Attach(out state);say("attach: "+state);if(link==null)return 2;
  try{for(int k=i+2;k<args.Length;k++){string a=args[k];if(a.StartsWith("wait ")){Thread.Sleep(int.Parse(a.Substring(5)));say(a);continue;}say("> "+a);say(link.Send(a));}return 0;}
  catch(Exception e){say("error: "+e);return 1;}finally{link.Dispose();}
 }

 [STAThread]static void Main(string[] args){
  Application.EnableVisualStyles();Application.SetCompatibleTextRenderingDefault(false);
  if(args.Contains("--selftest")){Environment.ExitCode=SelfTest(args);return;}
  bool first;using(var mutex=new Mutex(true,args.Contains("--preview")?"Local\\CronosTrainer.preview":"Local\\CronosTrainer.v1",out first)){
   if(!first){MessageBox.Show(L.T("Cronos Trainer уже запущен."),"Cronos Trainer");return;}
   try{Sounds.Init();Preview=args.Contains("--preview");var form=new Trainer();int li=Array.IndexOf(args,"--lang");if(li>=0 && li+1<args.Length)form.SetLanguage(args[li+1]=="en");int si=Array.IndexOf(args,"--shot");
    if(si>=0 && si+1<args.Length){string shot=Path.GetFullPath(args[si+1]);form.ClientSize=new Size(1000,1080);var t=new System.Windows.Forms.Timer{Interval=3000};
     t.Tick+=delegate{t.Stop();using(var b=new Bitmap(form.Width,form.Height)){form.AutoScrollPosition=new Point(0,0);form.DrawToBitmap(b,new Rectangle(0,0,form.Width,form.Height));b.Save(shot);}form.Close();};
     form.Shown+=delegate{t.Start();};}
    Application.Run(form);}catch(Exception e){MessageBox.Show(e.Message,L.T("Cronos Trainer: ошибка"));Environment.ExitCode=1;}
  }
 }
}

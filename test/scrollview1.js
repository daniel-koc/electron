// WebBrowserViews in scroll, use setContentBaseView for BaseWindow

const path = require("path");
const { app, BaseWindow, ScrollView, View, WebContentsView } = require("electron");

global.win = null;

function createWindow () {
  win = new BaseWindow({ width: 800, height: 400 })

  const scroll = new ScrollView()
  win.contentView.addChildView(scroll)
  scroll.setBounds({ x: 0, y: 0, width: 800, height: 400 })

  const scrollContent = new View()
  scroll.setContentView(scrollContent)
  scrollContent.setBounds({ x: 0, y: 0, width: 1200, height: 600 })

  const view1 = new WebContentsView()
  scrollContent.addChildView(view1)
  view1.webContents.loadURL('https://github.com')
  view1.setBounds({ x: 0, y: 0, width: 600, height: 600 })

  const view2 = new WebContentsView()
  scrollContent.addChildView(view2)
  view2.webContents.loadURL('https://bitbucket.org')
  view2.setBounds({ x: 600, y: 0, width: 600, height: 600 })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(() => {
  createWindow();

  app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (BaseWindow.getAllWindows().length === 0)
      createWindow();
  });
});

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bar to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', function () {
  if (process.platform !== 'darwin')
    app.quit();
});

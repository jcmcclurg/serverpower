<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class FRM1
    Inherits System.Windows.Forms.Form

    'Form overrides dispose to clean up the component list.
    <System.Diagnostics.DebuggerNonUserCode()> _
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Required by the Windows Form Designer
    Private components As System.ComponentModel.IContainer

    'NOTE: The following procedure is required by the Windows Form Designer
    'It can be modified using the Windows Form Designer.  
    'Do not modify it using the code editor.
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Dim resources As System.ComponentModel.ComponentResourceManager = New System.ComponentModel.ComponentResourceManager(GetType(FRM1))
        Me.LBL0 = New System.Windows.Forms.Label()
        Me.BTN1 = New System.Windows.Forms.Button()
        Me.BTN2 = New System.Windows.Forms.Button()
        Me.BTN3 = New System.Windows.Forms.Button()
        Me.GRP1 = New System.Windows.Forms.GroupBox()
        Me.CHK4 = New System.Windows.Forms.CheckBox()
        Me.CHK3 = New System.Windows.Forms.CheckBox()
        Me.CHK2 = New System.Windows.Forms.CheckBox()
        Me.CHK1 = New System.Windows.Forms.CheckBox()
        Me.TXT2 = New System.Windows.Forms.TextBox()
        Me.TXT1 = New System.Windows.Forms.TextBox()
        Me.BTN5 = New System.Windows.Forms.Button()
        Me.LBL2 = New System.Windows.Forms.Label()
        Me.BTN4 = New System.Windows.Forms.Button()
        Me.LBL1 = New System.Windows.Forms.Label()
        Me.LBL3 = New System.Windows.Forms.Label()
        Me.LBL4 = New System.Windows.Forms.Label()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.BTN6 = New System.Windows.Forms.Button()
        Me.GRP1.SuspendLayout()
        Me.SuspendLayout()
        '
        'LBL0
        '
        Me.LBL0.AutoSize = True
        Me.LBL0.Font = New System.Drawing.Font("Times New Roman", 16.2!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LBL0.Location = New System.Drawing.Point(156, 19)
        Me.LBL0.Name = "LBL0"
        Me.LBL0.Size = New System.Drawing.Size(415, 32)
        Me.LBL0.TabIndex = 0
        Me.LBL0.Text = "Agilent InfiniiVision Oscilloscope"
        '
        'BTN1
        '
        Me.BTN1.Font = New System.Drawing.Font("Times New Roman", 13.8!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.BTN1.Location = New System.Drawing.Point(411, 346)
        Me.BTN1.Name = "BTN1"
        Me.BTN1.Size = New System.Drawing.Size(200, 57)
        Me.BTN1.TabIndex = 1
        Me.BTN1.Text = "Capture"
        Me.BTN1.UseVisualStyleBackColor = True
        '
        'BTN2
        '
        Me.BTN2.Font = New System.Drawing.Font("Times New Roman", 13.8!, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.BTN2.Location = New System.Drawing.Point(411, 484)
        Me.BTN2.Name = "BTN2"
        Me.BTN2.Size = New System.Drawing.Size(200, 57)
        Me.BTN2.TabIndex = 2
        Me.BTN2.Text = "Save to File"
        Me.BTN2.UseVisualStyleBackColor = True
        '
        'BTN3
        '
        Me.BTN3.Location = New System.Drawing.Point(57, 382)
        Me.BTN3.Name = "BTN3"
        Me.BTN3.Size = New System.Drawing.Size(190, 47)
        Me.BTN3.TabIndex = 3
        Me.BTN3.Text = "Initialize"
        Me.BTN3.UseVisualStyleBackColor = True
        '
        'GRP1
        '
        Me.GRP1.Controls.Add(Me.CHK4)
        Me.GRP1.Controls.Add(Me.CHK3)
        Me.GRP1.Controls.Add(Me.CHK2)
        Me.GRP1.Controls.Add(Me.CHK1)
        Me.GRP1.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.GRP1.Location = New System.Drawing.Point(57, 85)
        Me.GRP1.Name = "GRP1"
        Me.GRP1.Size = New System.Drawing.Size(197, 212)
        Me.GRP1.TabIndex = 4
        Me.GRP1.TabStop = False
        Me.GRP1.Text = "Select a Channel"
        '
        'CHK4
        '
        Me.CHK4.AutoSize = True
        Me.CHK4.Location = New System.Drawing.Point(22, 164)
        Me.CHK4.Name = "CHK4"
        Me.CHK4.Size = New System.Drawing.Size(111, 26)
        Me.CHK4.TabIndex = 6
        Me.CHK4.Text = "Channel 4"
        Me.CHK4.UseVisualStyleBackColor = True
        '
        'CHK3
        '
        Me.CHK3.AutoSize = True
        Me.CHK3.Location = New System.Drawing.Point(22, 123)
        Me.CHK3.Name = "CHK3"
        Me.CHK3.Size = New System.Drawing.Size(111, 26)
        Me.CHK3.TabIndex = 6
        Me.CHK3.Text = "Channel 3"
        Me.CHK3.UseVisualStyleBackColor = True
        '
        'CHK2
        '
        Me.CHK2.AutoSize = True
        Me.CHK2.Location = New System.Drawing.Point(22, 80)
        Me.CHK2.Name = "CHK2"
        Me.CHK2.Size = New System.Drawing.Size(111, 26)
        Me.CHK2.TabIndex = 6
        Me.CHK2.Text = "Channel 2"
        Me.CHK2.UseVisualStyleBackColor = True
        '
        'CHK1
        '
        Me.CHK1.AutoSize = True
        Me.CHK1.Location = New System.Drawing.Point(22, 38)
        Me.CHK1.Name = "CHK1"
        Me.CHK1.Size = New System.Drawing.Size(111, 26)
        Me.CHK1.TabIndex = 6
        Me.CHK1.Text = "Channel 1"
        Me.CHK1.UseVisualStyleBackColor = True
        '
        'TXT2
        '
        Me.TXT2.Location = New System.Drawing.Point(528, 135)
        Me.TXT2.Name = "TXT2"
        Me.TXT2.Size = New System.Drawing.Size(139, 34)
        Me.TXT2.TabIndex = 5
        '
        'TXT1
        '
        Me.TXT1.Location = New System.Drawing.Point(310, 135)
        Me.TXT1.Name = "TXT1"
        Me.TXT1.Size = New System.Drawing.Size(139, 34)
        Me.TXT1.TabIndex = 6
        '
        'BTN5
        '
        Me.BTN5.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.BTN5.Location = New System.Drawing.Point(515, 208)
        Me.BTN5.Name = "BTN5"
        Me.BTN5.Size = New System.Drawing.Size(169, 39)
        Me.BTN5.TabIndex = 7
        Me.BTN5.Text = "Set Timebase"
        Me.BTN5.UseVisualStyleBackColor = True
        '
        'LBL2
        '
        Me.LBL2.AutoSize = True
        Me.LBL2.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LBL2.Location = New System.Drawing.Point(515, 72)
        Me.LBL2.Name = "LBL2"
        Me.LBL2.Size = New System.Drawing.Size(169, 44)
        Me.LBL2.TabIndex = 8
        Me.LBL2.Text = "Enter a timebase for" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "all channels (s/ div)"
        Me.LBL2.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'BTN4
        '
        Me.BTN4.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.BTN4.Location = New System.Drawing.Point(296, 208)
        Me.BTN4.Name = "BTN4"
        Me.BTN4.Size = New System.Drawing.Size(181, 39)
        Me.BTN4.TabIndex = 9
        Me.BTN4.Text = "Set Vertical Range"
        Me.BTN4.UseVisualStyleBackColor = True
        '
        'LBL1
        '
        Me.LBL1.AutoSize = True
        Me.LBL1.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LBL1.Location = New System.Drawing.Point(260, 72)
        Me.LBL1.Name = "LBL1"
        Me.LBL1.Size = New System.Drawing.Size(236, 44)
        Me.LBL1.TabIndex = 10
        Me.LBL1.Text = "Enter a vertical range for the" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "selected channel(s) (v/ div)"
        Me.LBL1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'LBL3
        '
        Me.LBL3.AutoSize = True
        Me.LBL3.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LBL3.Location = New System.Drawing.Point(75, 314)
        Me.LBL3.Name = "LBL3"
        Me.LBL3.Size = New System.Drawing.Size(165, 44)
        Me.LBL3.TabIndex = 11
        Me.LBL3.Text = "Select each channel" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "to be initialized"
        Me.LBL3.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'LBL4
        '
        Me.LBL4.AutoSize = True
        Me.LBL4.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.LBL4.Location = New System.Drawing.Point(368, 285)
        Me.LBL4.Name = "LBL4"
        Me.LBL4.Size = New System.Drawing.Size(283, 44)
        Me.LBL4.TabIndex = 12
        Me.LBL4.Text = "After initializing desired channels," & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "click to capture data"
        Me.LBL4.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Font = New System.Drawing.Font("Times New Roman", 12.0!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Label1.Location = New System.Drawing.Point(438, 421)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(142, 44)
        Me.Label1.TabIndex = 13
        Me.Label1.Text = "Save only one" & Global.Microsoft.VisualBasic.ChrW(13) & Global.Microsoft.VisualBasic.ChrW(10) & "channel at a time"
        Me.Label1.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
        '
        'BTN6
        '
        Me.BTN6.Location = New System.Drawing.Point(57, 494)
        Me.BTN6.Name = "BTN6"
        Me.BTN6.Size = New System.Drawing.Size(190, 47)
        Me.BTN6.TabIndex = 14
        Me.BTN6.Text = "Save and Exit"
        Me.BTN6.UseVisualStyleBackColor = True
        '
        'FRM1
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(13.0!, 26.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(717, 573)
        Me.Controls.Add(Me.BTN6)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.LBL4)
        Me.Controls.Add(Me.LBL3)
        Me.Controls.Add(Me.LBL1)
        Me.Controls.Add(Me.BTN4)
        Me.Controls.Add(Me.LBL2)
        Me.Controls.Add(Me.BTN5)
        Me.Controls.Add(Me.TXT1)
        Me.Controls.Add(Me.TXT2)
        Me.Controls.Add(Me.GRP1)
        Me.Controls.Add(Me.BTN3)
        Me.Controls.Add(Me.BTN2)
        Me.Controls.Add(Me.BTN1)
        Me.Controls.Add(Me.LBL0)
        Me.Font = New System.Drawing.Font("Times New Roman", 13.8!, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, CType(0, Byte))
        Me.Icon = CType(resources.GetObject("$this.Icon"), System.Drawing.Icon)
        Me.Margin = New System.Windows.Forms.Padding(5)
        Me.Name = "FRM1"
        Me.Text = "Multiple Channel Oscilloscope Acquisition"
        Me.GRP1.ResumeLayout(False)
        Me.GRP1.PerformLayout()
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents LBL0 As System.Windows.Forms.Label
    Friend WithEvents BTN1 As System.Windows.Forms.Button
    Friend WithEvents BTN2 As System.Windows.Forms.Button
    Friend WithEvents BTN3 As System.Windows.Forms.Button
    Friend WithEvents GRP1 As System.Windows.Forms.GroupBox
    Friend WithEvents TXT2 As System.Windows.Forms.TextBox
    Friend WithEvents CHK4 As System.Windows.Forms.CheckBox
    Friend WithEvents CHK3 As System.Windows.Forms.CheckBox
    Friend WithEvents CHK2 As System.Windows.Forms.CheckBox
    Friend WithEvents CHK1 As System.Windows.Forms.CheckBox
    Friend WithEvents TXT1 As System.Windows.Forms.TextBox
    Friend WithEvents BTN5 As System.Windows.Forms.Button
    Friend WithEvents LBL2 As System.Windows.Forms.Label
    Friend WithEvents BTN4 As System.Windows.Forms.Button
    Friend WithEvents LBL1 As System.Windows.Forms.Label
    Friend WithEvents LBL3 As System.Windows.Forms.Label
    Friend WithEvents LBL4 As System.Windows.Forms.Label
    Friend WithEvents Label1 As System.Windows.Forms.Label
    Friend WithEvents BTN6 As System.Windows.Forms.Button

End Class

REM Aydan Wynos
REM
REM This program is designed to connect to and read data from a
REM 3000 series InfiniiVision Digital Storage Oscilloscope. It will then send
REM the data to a binary file for analysis by the user.

Imports System.IO
Imports Ivi.Visa.Interop

Public Class FRM1

    Private scopeData As StreamWriter
    Private myMgr As ResourceManager
    Private myScope As FormattedIO488
    Private adjust As Double, offset As Double
    Private startTime As String

    Private Sub FRM1_Load(sender As Object, e As EventArgs) Handles MyBase.Load

        myMgr = New ResourceManager
        myScope = New FormattedIO488

        myScope.IO = myMgr.Open("USB0::2391::5965::MY50340366::INSTR")

        scopeData = File.CreateText("C:\Users\AWynos\Desktop\Oscilloscope Program\ScopeData.bin")

    End Sub

    Private Sub BTN1_Click(sender As Object, e As EventArgs) Handles BTN1.Click

        myScope.WriteString(":Autoscale")

        offset = 0
        adjust = 0

        TXT2.Clear()
        TXT1.Clear()

    End Sub

    Private Sub BTN3_Click(sender As Object, e As EventArgs) Handles BTN3.Click

        Dim chanRng As String, chanRange As String

        chanRng = TXT2.Text * 8

        chanRange = ":Channel1:Range " + chanRng

        myScope.WriteString(chanRange)

    End Sub

    Private Sub BTN4_Click(sender As Object, e As EventArgs) Handles BTN4.Click

        Dim rng As String, range As String

        rng = TXT1.Text * 10

        range = ":Timebase:Range " + rng

        myScope.WriteString(range)

    End Sub

    Private Sub BTN2_Click(sender As Object, e As EventArgs) Handles BTN7.Click

        Dim change As String, result

        myScope.WriteString(":Timebase:Delay?")

        result = myScope.ReadString

        adjust = result - 0.000125
        change = adjust.ToString

        myScope.WriteString(":Timebase:Delay " + change)

    End Sub

    Private Sub BTN5_Click(sender As Object, e As EventArgs) Handles BTN5.Click

        Dim change As String, result As String

        myScope.WriteString(":Channel1:Offset?")

        result = myScope.ReadString

        offset = result - 0.1
        change = offset.ToString

        myScope.WriteString(":Channel1:Offset " + change)

    End Sub

    Private Sub BTN6_Click(sender As Object, e As EventArgs) Handles BTN6.Click

        Dim change As String, result As String

        myScope.WriteString(":Channel1:Offset?")

        result = myScope.ReadString

        offset = result + 0.1
        change = offset.ToString

        myScope.WriteString(":Channel1:Offset " + change)

    End Sub

    Private Sub BTN8_Click(sender As Object, e As EventArgs) Handles BTN8.Click

        Dim change As String, result As String

        myScope.WriteString(":Timebase:Delay?")

        result = myScope.ReadString

        adjust = result + 0.000125
        change = adjust.ToString

        myScope.WriteString(":Timebase:Delay " + change)

    End Sub

    Private Sub getTime()

        If DateTime.Now.Hour < 10 Then
            startTime += "0"
        End If

        startTime += DateTime.Now.ToString("HH:mm:ss")

        startTime += ":"

        If DateTime.Now.Millisecond < 100 Then
            startTime += "0"
        End If

        startTime += DateTime.Now.Millisecond.ToString

    End Sub

    Private Sub BTN9_Click(sender As Object, e As EventArgs) Handles BTN9.Click

        myScope.WriteString(":Timebase:Mode Main")
        myScope.WriteString(":Acquire:Type Normal")
        myScope.WriteString(":Acquire:Complete 100")

        myScope.WriteString(":Waveform:Source channel1")
        myScope.WriteString(":Waveform:Format BYTE")
        myScope.WriteString(":Waveform:Points MAXimum")

        getTime()

        myScope.WriteString(":Digitize Channel1")

    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles BTN10.Click

        Dim result As String = ""

        myScope.WriteString(":Waveform:Data?")

        result += startTime
        result += myScope.ReadString

        scopeData.WriteLine(result)

    End Sub

    Private Sub BTN11_Click(sender As Object, e As EventArgs) Handles BTN11.Click

        scopeData.Close()

        End

    End Sub

    Private Sub BTN12_Click(sender As Object, e As EventArgs) Handles BTN12.Click

        MsgBox(DateTime.Now.ToString("HH:mm:ss"))

    End Sub

    Private Sub BTN13_Click(sender As Object, e As EventArgs) Handles BTN13.Click

        Dim result As String

        myScope.WriteString(":Save:Waveform:Format BINary")
        myScope.WriteString(":Save:Waveform:Length MAXimum")
        myScope.WriteString(":Save:Filename ""Scope1""")

        myScope.WriteString(":Save:Waveform:[STARt][""Scope1""]")

        'result = myScope.ReadString

        'MsgBox(result)

    End Sub

End Class

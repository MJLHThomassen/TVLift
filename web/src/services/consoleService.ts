import { injectable, inject } from "tsyringe";
import { IWebsocketService } from "@/services/iWebsocketService";

import * as Ansicolor from "ansicolor";

export const consoleLevels = ["Verbose", "Debug", "Info", "Warning", "Error"] as const;
export type ConsoleLevels = typeof consoleLevels[number];

export interface ConsoleLevel
{
    idx: number;
    str: ConsoleLevels;
}

export interface ConsoleEntry
{
  index: number;
  level: ConsoleLevel;
  text: string;
}

@injectable()
export class ConsoleService
{
    public readonly entries: ConsoleEntry[] = [];
    public readonly scrollback = 1000;

    constructor(
        @inject("IWebsocketService")
        private websocketService: IWebsocketService)
    {
        this.websocketService.onMessageRecieved((event) => 
        {
            this.addConsoleLines(event.data);
        });
    }

    private async addConsoleLines(text: string): Promise<void>
    {
        let level: ConsoleLevels;
        switch(Ansicolor.strip(text)[0].toUpperCase())
        {
            case 'V':
                level = "Verbose";
                break;
            case 'D':
                level = "Debug";
                break;
            case 'W':
                level = "Warning";
                break;
            case 'E':
                level = "Error";
                break;
            case 'I':
            default:
                level = "Info";
                break;
        }
    
        const newLine = 
        {
            index: this.entries.length,
            level: {
                str: level,
                idx: consoleLevels.findIndex(x => x === level),
            },
            text: ` > ${text}\r`
        };

        if(this.entries.length == this.scrollback)
        {
            this.entries.shift();
        }

        this.entries.push(newLine);
    }
}
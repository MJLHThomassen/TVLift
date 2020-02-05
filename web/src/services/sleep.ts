export function sleep(ms: number): Promise<void>
{
    return new Promise((resolve) => setTimeout(resolve, ms));
}

export function timeout<T>(ms: number): Promise<T>
{
    return new Promise<T>((resolve, reject) => setTimeout(reject, ms));
}

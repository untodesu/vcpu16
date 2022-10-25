/* SPDX-License-Identifier: BSD-2-Clause */
/* 
 * Copyright (c) 2022, Kirill GPRB
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <dvcpu.hpp>
#include <iostream>

int main()
{
    dvcpu::Instruction instr;
    dvcpu::CPU cpu = dvcpu::CPU(25000);

    cpu.attach(std::make_shared<dvcpu::Memory>(dvcpu::ADDRESS_LIMIT));

    cpu.memory->write(0x0000, 0x7A01);  // movii $imm, %R0
    cpu.memory->write(0x0001, 0xABCD);  // imm = 0xABCD
    cpu.memory->write(0x0002, 0x0400);  // halt

    dvcpu::Error error;
    while((error = cpu.tick()) == dvcpu::Error::OK);

    std::cout << "ERROR " << std::hex << (unsigned int)error << std::endl;
    std::cout << "R0 = 0x" << std::hex << cpu.registers[dvcpu::REG_R0] << std::endl;
    std::cout << "RI = 0x" << std::hex << cpu.registers[dvcpu::REG_RI] << std::endl;

    return 0;
}

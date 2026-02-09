import { useEffect, useRef } from 'react';
import './GridLayout.css';

const GridLayout = ({ floor, path, highlightSlot }) => {
  const canvasRef = useRef(null);
  
  useEffect(() => {
    if (!floor) return;
    
    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    const cellSize = 40;
    
    // Find grid bounds
    let maxX = 0, maxY = 0;
    floor.slots.forEach(slot => {
      maxX = Math.max(maxX, slot.x);
      maxY = Math.max(maxY, slot.y);
    });
    
    canvas.width = (maxX + 2) * cellSize;
    canvas.height = (maxY + 2) * cellSize;
    
    // Clear canvas
    ctx.fillStyle = '#1e1e2e';
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    // Draw grid lines
    ctx.strokeStyle = '#2a2a3e';
    ctx.lineWidth = 1;
    for (let x = 0; x <= maxX + 1; x++) {
      ctx.beginPath();
      ctx.moveTo(x * cellSize, 0);
      ctx.lineTo(x * cellSize, canvas.height);
      ctx.stroke();
    }
    for (let y = 0; y <= maxY + 1; y++) {
      ctx.beginPath();
      ctx.moveTo(0, y * cellSize);
      ctx.lineTo(canvas.width, y * cellSize);
      ctx.stroke();
    }
    
    // Draw path if exists
    if (path && path.length > 0) {
      ctx.strokeStyle = '#fbbf24';
      ctx.lineWidth = 3;
      ctx.beginPath();
      ctx.moveTo(path[0].x * cellSize + cellSize/2, path[0].y * cellSize + cellSize/2);
      path.forEach((point, i) => {
        if (i > 0) {
          ctx.lineTo(point.x * cellSize + cellSize/2, point.y * cellSize + cellSize/2);
        }
      });
      ctx.stroke();
      
      // Draw path nodes
      path.forEach((point, i) => {
        ctx.fillStyle = i === 0 ? '#10b981' : i === path.length - 1 ? '#3b82f6' : '#fbbf24';
        ctx.beginPath();
        ctx.arc(point.x * cellSize + cellSize/2, point.y * cellSize + cellSize/2, 5, 0, Math.PI * 2);
        ctx.fill();
      });
    }
    
    // Draw slots
    floor.slots.forEach(slot => {
      const x = slot.x * cellSize;
      const y = slot.y * cellSize;
      
      // Slot background
      if (slot.id === highlightSlot) {
        ctx.fillStyle = '#6366f1';
      } else if (slot.is_occupied) {
        ctx.fillStyle = '#ef4444';
      } else {
        ctx.fillStyle = '#10b981';
      }
      
      ctx.fillRect(x + 5, y + 5, cellSize - 10, cellSize - 10);
      
      // Slot border
      ctx.strokeStyle = '#fff';
      ctx.lineWidth = 2;
      ctx.strokeRect(x + 5, y + 5, cellSize - 10, cellSize - 10);
      
      // Slot ID text
      ctx.fillStyle = '#fff';
      ctx.font = 'bold 10px Arial';
      ctx.textAlign = 'center';
      ctx.fillText(slot.id, x + cellSize/2, y + cellSize/2 + 3);
    });
    
    // Draw entry marker
    ctx.fillStyle = '#10b981';
    ctx.font = 'bold 14px Arial';
    ctx.fillText('ENTRY', cellSize/2, cellSize/2);
    
  }, [floor, path, highlightSlot]);
  
  return (
    <div className="grid-layout">
      <canvas ref={canvasRef} />
    </div>
  );
};

export default GridLayout;

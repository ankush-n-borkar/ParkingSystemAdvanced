import { useEffect, useRef, useState } from 'react';
import axios from 'axios';
import { ArrowLeft, AlertCircle, Layers } from 'lucide-react';
import './VisualizationPage.css';

const VisualizationPage = ({ parkingData, onBack }) => {
  const canvasRef = useRef(null);
  const [selectedFloor, setSelectedFloor] = useState(0);
  const [selectedSlot, setSelectedSlot] = useState(null);
  const [hoveredSlot, setHoveredSlot] = useState(null);
  const [path, setPath] = useState(null);
  const [pathDistance, setPathDistance] = useState(0);

  const CELL_SIZE = 60;
  const PADDING = 40;

  useEffect(() => {
    if (!parkingData) return;
    drawLayout();
  }, [parkingData, selectedFloor, selectedSlot, hoveredSlot, path]);

const drawLayout = () => {
  const canvas = canvasRef.current;
  if (!canvas) return;

  const ctx = canvas.getContext('2d');
  const floor = parkingData.floors[selectedFloor];
  
  const maxX = 8;
  const maxY = 10;  // Compact layout
  canvas.width = maxX * CELL_SIZE + PADDING * 2;
  canvas.height = maxY * CELL_SIZE + PADDING * 2;

  // Background
  ctx.fillStyle = '#0f172a';
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  // Grid
  ctx.strokeStyle = '#1e293b';
  ctx.lineWidth = 1;
  for (let x = 0; x <= maxX; x++) {
    ctx.beginPath();
    ctx.moveTo(PADDING + x * CELL_SIZE, PADDING);
    ctx.lineTo(PADDING + x * CELL_SIZE, PADDING + maxY * CELL_SIZE);
    ctx.stroke();
  }
  for (let y = 0; y <= maxY; y++) {
    ctx.beginPath();
    ctx.moveTo(PADDING, PADDING + y * CELL_SIZE);
    ctx.lineTo(PADDING + maxX * CELL_SIZE, PADDING + y * CELL_SIZE);
    ctx.stroke();
  }

  // ========== DRAW ROADS ==========
  ctx.fillStyle = '#374151';
  
  // Left vertical road (main entry/exit corridor)
  for (let y = 0; y <= 9; y++) {
    ctx.fillRect(PADDING, PADDING + y * CELL_SIZE, CELL_SIZE, CELL_SIZE);
  }
  
  // Horizontal road between Row 1 and Row 2 (Y=2)
  for (let x = 1; x <= 6; x++) {
    ctx.fillRect(PADDING + x * CELL_SIZE, PADDING + 2 * CELL_SIZE, CELL_SIZE, CELL_SIZE);
  }
  
  // Horizontal road between Row 3 and Row 4 (Y=6) - CORRECTED POSITION
  for (let x = 1; x <= 6; x++) {
    ctx.fillRect(PADDING + x * CELL_SIZE, PADDING + 6 * CELL_SIZE, CELL_SIZE, CELL_SIZE);
  }

  // Road borders
  ctx.strokeStyle = '#4b5563';
  ctx.lineWidth = 2;
  ctx.strokeRect(PADDING, PADDING, CELL_SIZE, 9 * CELL_SIZE);

  // Yellow center lines
  ctx.strokeStyle = '#fbbf24';
  ctx.lineWidth = 2;
  ctx.setLineDash([10, 10]);
  
  // Vertical road center line
  ctx.beginPath();
  ctx.moveTo(PADDING + CELL_SIZE / 2, PADDING);
  ctx.lineTo(PADDING + CELL_SIZE / 2, PADDING + 9 * CELL_SIZE);
  ctx.stroke();
  
  // Horizontal road markings
  [2, 6].forEach(y => {
    ctx.beginPath();
    ctx.moveTo(PADDING + CELL_SIZE, PADDING + y * CELL_SIZE + CELL_SIZE / 2);
    ctx.lineTo(PADDING + 7 * CELL_SIZE, PADDING + y * CELL_SIZE + CELL_SIZE / 2);
    ctx.stroke();
  });
  
  ctx.setLineDash([]);

  // Draw path
  if (path && path.length > 0) {
    drawPathOnRoads(ctx, path);
  }

  // Entry gate
  drawGateBox(ctx, PADDING, PADDING, 'ENTRY', '#10b981', '↓');
  
  // Exit gate
  drawGateBox(ctx, PADDING, PADDING + 9 * CELL_SIZE, 'EXIT', '#ef4444', '↑');

  // Draw parking slots with CORRECTED LAYOUT
  floor.slots.forEach((slot, idx) => {
    let gridX, gridY;
    
    // Row 1: Slots 0-5 (R1S1-R1S6) at Y=1
    if (idx >= 0 && idx < 6) {
      gridX = idx + 1;
      gridY = 1;
    }
    // Row 2: Slots 6-11 (R2S1-R2S6) at Y=3
    else if (idx >= 6 && idx < 12) {
      gridX = idx - 6 + 1;
      gridY = 3;
    }
    // Row 3: Slots 12-17 (R3S1-R3S6) at Y=5 (small gap after Y=3)
    else if (idx >= 12 && idx < 18) {
      gridX = idx - 12 + 1;
      gridY = 5;
    }
    // Row 4: Slots 18-23 (R4S1-R4S6) at Y=7 (after road at Y=6)
    else if (idx >= 18 && idx < 24) {
      gridX = idx - 18 + 1;
      gridY = 7;
    }
    // Additional rows
    else if (idx >= 24 && idx < 30) {
      gridX = idx - 24 + 1;
      gridY = 8;
    }
    else if (idx >= 30 && idx < 36) {
      gridX = idx - 30 + 1;
      gridY = 9;
    }
    else {
      return;
    }
    
    drawParkingSlotAtPosition(ctx, slot, gridX, gridY);
  });

  // Section labels
  ctx.fillStyle = '#6b7280';
  ctx.font = 'bold 11px Arial';
  ctx.textAlign = 'right';
  ctx.fillText('Row 1-2', PADDING - 10, PADDING + 2 * CELL_SIZE + CELL_SIZE / 2);
  
  ctx.fillStyle = '#94a3b8';
  ctx.font = '9px Arial';
  ctx.fillText('gap', PADDING - 10, PADDING + 4.2 * CELL_SIZE);
  
  ctx.fillStyle = '#6b7280';
  ctx.font = 'bold 11px Arial';
  ctx.fillText('Row 3-4', PADDING - 10, PADDING + 6.5 * CELL_SIZE);
};


const drawParkingSlotAtPosition = (ctx, slot, gridX, gridY) => {
  const x = PADDING + gridX * CELL_SIZE;
  const y = PADDING + gridY * CELL_SIZE;
  const margin = 8;
  
  let bgColor, borderColor;
  if (selectedSlot === slot.id) {
    bgColor = '#6366f1';
    borderColor = '#4f46e5';
  } else if (hoveredSlot === slot.id) {
    bgColor = '#818cf8';
    borderColor = '#6366f1';
  } else if (slot.is_occupied) {
    bgColor = '#ef4444';
    borderColor = '#dc2626';
  } else {
    bgColor = '#10b981';
    borderColor = '#059669';
  }

  const gradient = ctx.createLinearGradient(x, y, x, y + CELL_SIZE);
  gradient.addColorStop(0, bgColor);
  gradient.addColorStop(1, borderColor);
  ctx.fillStyle = gradient;
  
  drawRoundedRect(ctx, x + margin, y + margin, CELL_SIZE - margin * 2, CELL_SIZE - margin * 2, 6);
  ctx.fill();
  
  ctx.strokeStyle = 'white';
  ctx.lineWidth = 2;
  ctx.stroke();

  ctx.fillStyle = 'white';
  ctx.font = 'bold 10px Arial';
  ctx.textAlign = 'center';
  ctx.shadowColor = 'rgba(0, 0, 0, 0.5)';
  ctx.shadowBlur = 3;
  
  const shortId = slot.id.split('-')[1] || slot.id;
  ctx.fillText(shortId, x + CELL_SIZE / 2, y + CELL_SIZE / 2 - 2);
  
  ctx.font = '8px Arial';
  ctx.fillText(slot.type === 'large' ? 'LARGE' : 'STD', x + CELL_SIZE / 2, y + CELL_SIZE / 2 + 10);
  ctx.shadowBlur = 0;

  if (slot.is_occupied && slot.vehicle_number) {
    ctx.font = 'bold 7px monospace';
    ctx.fillText(slot.vehicle_number, x + CELL_SIZE / 2, y + CELL_SIZE / 2 + 20);
  } else {
    ctx.font = 'bold 14px Arial';
    ctx.fillText('P', x + CELL_SIZE / 2, y + CELL_SIZE - 8);
  }
};


  const drawGateBox = (ctx, x, y, label, color, arrow) => {
    const gradient = ctx.createLinearGradient(x, y, x, y + CELL_SIZE);
    gradient.addColorStop(0, color);
    gradient.addColorStop(1, color + 'cc');
    ctx.fillStyle = gradient;
    
    ctx.fillRect(x + 8, y + 8, CELL_SIZE - 16, CELL_SIZE - 16);
    
    ctx.strokeStyle = 'white';
    ctx.lineWidth = 3;
    ctx.strokeRect(x + 8, y + 8, CELL_SIZE - 16, CELL_SIZE - 16);
    
    ctx.fillStyle = 'white';
    ctx.font = 'bold 10px Arial';
    ctx.textAlign = 'center';
    ctx.shadowColor = 'rgba(0,0,0,0.5)';
    ctx.shadowBlur = 4;
    ctx.fillText(label, x + CELL_SIZE / 2, y + CELL_SIZE / 2 - 2);
    
    ctx.font = 'bold 16px Arial';
    ctx.fillText(arrow, x + CELL_SIZE / 2, y + CELL_SIZE / 2 + 14);
    ctx.shadowBlur = 0;
  };

  const drawPathOnRoads = (ctx, pathPoints) => {
    if (pathPoints.length < 2) return;

    ctx.strokeStyle = 'rgba(251, 191, 36, 0.3)';
    ctx.lineWidth = 20;
    ctx.lineCap = 'round';
    ctx.lineJoin = 'round';
    
    ctx.beginPath();
    pathPoints.forEach((point, i) => {
      const x = PADDING + point.x * CELL_SIZE + CELL_SIZE / 2;
      const y = PADDING + point.y * CELL_SIZE + CELL_SIZE / 2;
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.stroke();

    ctx.strokeStyle = '#fbbf24';
    ctx.lineWidth = 6;
    ctx.shadowColor = '#fbbf24';
    ctx.shadowBlur = 12;
    
    ctx.beginPath();
    pathPoints.forEach((point, i) => {
      const x = PADDING + point.x * CELL_SIZE + CELL_SIZE / 2;
      const y = PADDING + point.y * CELL_SIZE + CELL_SIZE / 2;
      if (i === 0) ctx.moveTo(x, y);
      else ctx.lineTo(x, y);
    });
    ctx.stroke();
    ctx.shadowBlur = 0;

    // Direction arrows
    for (let i = 0; i < pathPoints.length - 1; i++) {
      const p1 = pathPoints[i];
      const p2 = pathPoints[i + 1];
      
      const x1 = PADDING + p1.x * CELL_SIZE + CELL_SIZE / 2;
      const y1 = PADDING + p1.y * CELL_SIZE + CELL_SIZE / 2;
      const x2 = PADDING + p2.x * CELL_SIZE + CELL_SIZE / 2;
      const y2 = PADDING + p2.y * CELL_SIZE + CELL_SIZE / 2;
      
      const midX = (x1 + x2) / 2;
      const midY = (y1 + y2) / 2;
      const angle = Math.atan2(y2 - y1, x2 - x1);
      
      ctx.save();
      ctx.translate(midX, midY);
      ctx.rotate(angle);
      
      ctx.fillStyle = 'white';
      ctx.beginPath();
      ctx.moveTo(10, 0);
      ctx.lineTo(-5, -6);
      ctx.lineTo(-5, 6);
      ctx.closePath();
      ctx.fill();
      
      ctx.restore();
    }
  };

  const drawParkingSlot = (ctx, slot) => {
    const x = PADDING + slot.x * CELL_SIZE;
    const y = PADDING + slot.y * CELL_SIZE;
    const margin = 8;
    
    let bgColor, borderColor;
    if (selectedSlot === slot.id) {
      bgColor = '#6366f1';
      borderColor = '#4f46e5';
    } else if (hoveredSlot === slot.id) {
      bgColor = '#818cf8';
      borderColor = '#6366f1';
    } else if (slot.is_occupied) {
      bgColor = '#ef4444';
      borderColor = '#dc2626';
    } else {
      bgColor = '#10b981';
      borderColor = '#059669';
    }

    const gradient = ctx.createLinearGradient(x, y, x, y + CELL_SIZE);
    gradient.addColorStop(0, bgColor);
    gradient.addColorStop(1, borderColor);
    ctx.fillStyle = gradient;
    
    drawRoundedRect(ctx, x + margin, y + margin, CELL_SIZE - margin * 2, CELL_SIZE - margin * 2, 6);
    ctx.fill();
    
    ctx.strokeStyle = 'white';
    ctx.lineWidth = 2;
    ctx.stroke();

    ctx.fillStyle = 'white';
    ctx.font = 'bold 10px Arial';
    ctx.textAlign = 'center';
    ctx.shadowColor = 'rgba(0, 0, 0, 0.5)';
    ctx.shadowBlur = 3;
    
    const shortId = slot.id.split('-')[1] || slot.id;
    ctx.fillText(shortId, x + CELL_SIZE / 2, y + CELL_SIZE / 2 - 2);
    
    ctx.font = '8px Arial';
    ctx.fillText(slot.type === 'large' ? 'LARGE' : 'STD', x + CELL_SIZE / 2, y + CELL_SIZE / 2 + 10);
    ctx.shadowBlur = 0;

    if (slot.is_occupied && slot.vehicle_number) {
      ctx.font = 'bold 7px monospace';
      ctx.fillText(slot.vehicle_number, x + CELL_SIZE / 2, y + CELL_SIZE / 2 + 20);
    } else {
      ctx.font = 'bold 14px Arial';
      ctx.fillText('P', x + CELL_SIZE / 2, y + CELL_SIZE - 8);
    }
  };

  const drawRoundedRect = (ctx, x, y, width, height, radius) => {
    ctx.beginPath();
    ctx.moveTo(x + radius, y);
    ctx.lineTo(x + width - radius, y);
    ctx.quadraticCurveTo(x + width, y, x + width, y + radius);
    ctx.lineTo(x + width, y + height - radius);
    ctx.quadraticCurveTo(x + width, y + height, x + width - radius, y + height);
    ctx.lineTo(x + radius, y + height);
    ctx.quadraticCurveTo(x, y + height, x, y + height - radius);
    ctx.lineTo(x, y + radius);
    ctx.quadraticCurveTo(x, y, x + radius, y);
    ctx.closePath();
  };

const handleCanvasClick = (e) => {
  const canvas = canvasRef.current;
  const rect = canvas.getBoundingClientRect();
  const x = e.clientX - rect.left - PADDING;
  const y = e.clientY - rect.top - PADDING;

  const gridX = Math.floor(x / CELL_SIZE);
  const gridY = Math.floor(y / CELL_SIZE);

  const floor = parkingData.floors[selectedFloor];
  
  // Find slot by matching grid position
  let clickedSlot = null;
  floor.slots.forEach((slot, idx) => {
    let slotGridX, slotGridY;
    
    if (idx >= 0 && idx < 6) {
      slotGridX = idx + 1;
      slotGridY = 1;
    } else if (idx >= 6 && idx < 12) {
      slotGridX = idx - 6 + 1;
      slotGridY = 3;
    } else if (idx >= 12 && idx < 18) {
      slotGridX = idx - 12 + 1;
      slotGridY = 5;
    } else if (idx >= 18 && idx < 24) {
      slotGridX = idx - 18 + 1;
      slotGridY = 7;
    } else if (idx >= 24 && idx < 30) {
      slotGridX = idx - 24 + 1;
      slotGridY = 9;
    } else if (idx >= 30 && idx < 36) {
      slotGridX = idx - 30 + 1;
      slotGridY = 10;
    } else if (idx >= 36 && idx < 42) {
      slotGridX = idx - 36 + 1;
      slotGridY = 11;
    }
    
    if (gridX === slotGridX && gridY === slotGridY) {
      clickedSlot = slot;
    }
  });

  if (clickedSlot) {
    setSelectedSlot(clickedSlot.id);
    calculatePath(clickedSlot);
  } else {
    setSelectedSlot(null);
    setPath(null);
  }
};


const handleCanvasMouseMove = (e) => {
  const canvas = canvasRef.current;
  const rect = canvas.getBoundingClientRect();
  const x = e.clientX - rect.left - PADDING;
  const y = e.clientY - rect.top - PADDING;

  const gridX = Math.floor(x / CELL_SIZE);
  const gridY = Math.floor(y / CELL_SIZE);

  const floor = parkingData.floors[selectedFloor];
  
  let hoveredSlot = null;
  floor.slots.forEach((slot, idx) => {
    let slotGridX, slotGridY;
    
    if (idx >= 0 && idx < 6) {
      slotGridX = idx + 1;
      slotGridY = 1;
    } else if (idx >= 6 && idx < 12) {
      slotGridX = idx - 6 + 1;
      slotGridY = 3;
    } else if (idx >= 12 && idx < 18) {
      slotGridX = idx - 12 + 1;
      slotGridY = 5;
    } else if (idx >= 18 && idx < 24) {
      slotGridX = idx - 18 + 1;
      slotGridY = 7;
    } else if (idx >= 24 && idx < 30) {
      slotGridX = idx - 24 + 1;
      slotGridY = 9;
    } else if (idx >= 30 && idx < 36) {
      slotGridX = idx - 30 + 1;
      slotGridY = 10;
    } else if (idx >= 36 && idx < 42) {
      slotGridX = idx - 36 + 1;
      slotGridY = 11;
    }
    
    if (gridX === slotGridX && gridY === slotGridY) {
      hoveredSlot = slot;
    }
  });

  setHoveredSlot(hoveredSlot ? hoveredSlot.id : null);
  canvas.style.cursor = hoveredSlot ? 'pointer' : 'default';
};


  const calculatePath = async (slot) => {
    try {
      const response = await axios.get(`/api/path?slot_id=${slot.id}`);
      
      if (response.data.status === 'success') {
        setPath(response.data.path);
        setPathDistance(response.data.path_length);
      }
    } catch (error) {
      console.error('Error calculating path:', error);
    }
  };

  if (!parkingData) {
    return <div>Loading...</div>;
  }

  return (
    <div className="visualization-page">
      <div className="viz-header">
        <button onClick={onBack} className="back-btn">
          <ArrowLeft size={20} />
          Back to Home
        </button>
        <div className="viz-title">
          <Layers size={28} />
          <h2>Interactive Parking Layout</h2>
        </div>
        <div className="floor-selector">
          {parkingData.floors.map((floor, idx) => (
            <button
              key={idx}
              className={`floor-btn ${selectedFloor === idx ? 'active' : ''}`}
              onClick={() => {
                setSelectedFloor(idx);
                setSelectedSlot(null);
                setPath(null);
              }}
            >
              <span className="floor-icon">🏢</span>
              {floor.name}
              <span className="floor-count">{floor.occupied_slots}/{floor.total_slots}</span>
            </button>
          ))}
        </div>
      </div>

      <div className="viz-content">
        <div className="canvas-container">
          <canvas
            ref={canvasRef}
            onClick={handleCanvasClick}
            onMouseMove={handleCanvasMouseMove}
          />
        </div>

        <div className="viz-sidebar">
          {selectedSlot && (
            <div className="slot-info-panel">
              <h3>📍 Selected Slot</h3>
              <div className="info-row">
                <span>Slot ID:</span>
                <strong>{selectedSlot}</strong>
              </div>
              {path && (
                <>
                  <div className="info-row">
                    <span>Distance:</span>
                    <strong>{pathDistance} steps</strong>
                  </div>
                  <div className="path-info">
                    <p>
                      🚗 Follow the <span style={{color: '#fbbf24', fontWeight: 'bold'}}>yellow path</span> from entry gate
                    </p>
                  </div>
                </>
              )}
            </div>
          )}

          <div className="legend">
            <h3>🎨 Legend</h3>
            <div className="legend-item">
              <div className="legend-color green"></div>
              <span>Available</span>
            </div>
            <div className="legend-item">
              <div className="legend-color red"></div>
              <span>Occupied</span>
            </div>
            <div className="legend-item">
              <div className="legend-color purple"></div>
              <span>Selected</span>
            </div>
            <div className="legend-item">
              <div className="legend-color yellow"></div>
              <span>Path</span>
            </div>
            <div className="legend-item">
              <div className="legend-color" style={{background: 'linear-gradient(135deg, #374151, #1f2937)'}}></div>
              <span>Road</span>
            </div>
          </div>

          <div className="tip">
            <AlertCircle size={16} />
            <span>Click any slot to see the BFS shortest path</span>
          </div>
        </div>
      </div>
    </div>
  );
};

export default VisualizationPage;

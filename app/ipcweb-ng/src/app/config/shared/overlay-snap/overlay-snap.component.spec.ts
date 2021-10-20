import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { OverlaySnapComponent } from './overlay-snap.component';

describe('OverlaySnapComponent', () => {
  let component: OverlaySnapComponent;
  let fixture: ComponentFixture<OverlaySnapComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ OverlaySnapComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(OverlaySnapComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
